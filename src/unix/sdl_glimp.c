/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
* SDL code from "ET: Legacy"
* Copyright (C) 2012 Jan Simek <mail@etlegacy.com>
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <signal.h>

#include <dlfcn.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "linux_local.h"

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_thread.h>

#define WINDOW_CLASS_NAME   "Return to Castle Wolfenstein"
#define WINDOW_CLASS_NAME_MIN "RtCW"
static SDL_Surface      *SDLvidscreen = NULL;
static const SDL_VideoInfo *videoInfo = NULL;

void(APIENTRYP qglActiveTextureARB) (GLenum texture);
void(APIENTRYP qglClientActiveTextureARB) (GLenum texture);
void(APIENTRYP qglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
void(APIENTRYP qglLockArraysEXT) (GLint first, GLsizei count);
void(APIENTRYP qglUnlockArraysEXT) (void);

typedef enum
{
	RSERR_OK,
	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,
	RSERR_UNKNOWN
} rserr_t;

char *Q_stristr( char *s, char *find ) {
	register char c, sc;
	register size_t len;

	if ( ( c = *find++ ) != 0 ) {
		if ( c >= 'a' && c <= 'z' ) {
			c -= ( 'a' - 'A' );
		}
		len = strlen( find );
		do
		{
			do
			{
				if ( ( sc = *s++ ) == 0 ) {
					return NULL;
				}
				if ( sc >= 'a' && sc <= 'z' ) {
					sc -= ( 'a' - 'A' );
				}
			} while ( sc != c );
		} while ( Q_stricmpn( s, find, len ) != 0 );
		s--;
	}
	return s;
}

static void install_grabs( void ) {
}
static void uninstall_grabs( void ) {
}
static void HandleEvents( void ) {
}
static qboolean signalcaught = qfalse;

void Sys_Exit( int );

static void signal_handler( int sig ) {
	if ( signalcaught ) {
		printf( "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n", sig );
		Sys_Exit( 1 );
	}
	signalcaught = qtrue;
	printf( "Received signal %d, exiting...\n", sig );
	GLimp_Shutdown();
	Sys_Exit( 0 );
}

static void InitSig( void ) {
	signal( SIGHUP, signal_handler );
	signal( SIGQUIT, signal_handler );
	signal( SIGILL, signal_handler );
	signal( SIGTRAP, signal_handler );
	signal( SIGIOT, signal_handler );
	signal( SIGBUS, signal_handler );
	signal( SIGFPE, signal_handler );
	signal( SIGSEGV, signal_handler );
	signal( SIGTERM, signal_handler );
}

void GLimp_Shutdown( void ) {
	IN_Shutdown();

	if ( glConfig.smpActive && !signalcaught )
	{
		// may already be dead if called from signal handler
		GLimp_WakeRenderer( (void *)0xdead );
		// - and wait for it to return
		GLimp_FrontEndSleep();
	}
	if ( SDLvidscreen )
		SDL_VideoQuit();
	if ( SDL_WasInit(SDL_INIT_VIDEO) )
		SDL_QuitSubSystem(SDL_INIT_VIDEO);

	GLimp_ShutdownRenderThread();
	SDLvidscreen = NULL;
	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

void GLimp_LogComment( char *comment ) {
}

int GLimp_SetMode(int mode, qboolean fullscreen, qboolean noborder)
{
	int         sdlcolorbits;
	int         colorbits, depthbits, stencilbits;
	int         tcolorbits, tdepthbits, tstencilbits;
	int         i          = 0;
	Uint32      flags      = SDL_OPENGL;

	ri.Printf(PRINT_ALL, "Initializing OpenGL display\n");

	if (videoInfo == NULL)
	{
		static SDL_VideoInfo   sVideoInfo;
		static SDL_PixelFormat sPixelFormat;

		videoInfo = SDL_GetVideoInfo();

		// Take a copy of the videoInfo
		Com_Memcpy(&sPixelFormat, videoInfo->vfmt, sizeof(SDL_PixelFormat));
		sPixelFormat.palette = NULL; // Should already be the case
		Com_Memcpy(&sVideoInfo, videoInfo, sizeof(SDL_VideoInfo));
		sVideoInfo.vfmt = &sPixelFormat;
		videoInfo       = &sVideoInfo;
	}



	if (mode == -2)
	{
		// use desktop video resolution
		if ( videoInfo->current_w > 0)
		{
			glConfig.vidWidth  = videoInfo->current_w;
			glConfig.vidHeight = videoInfo->current_h;
		}
		else
		{
			glConfig.vidWidth  = 800;
			glConfig.vidHeight = 600;
			ri.Printf(PRINT_ALL,
			          "Cannot determine display resolution, assuming 800x600\n");
		}

		glConfig.windowAspect = (float)glConfig.vidWidth / (float)glConfig.vidHeight;
	}
	else if (!R_GetModeInfo(&glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode))
	{
		ri.Printf(PRINT_ALL, " invalid mode\n");
		return RSERR_INVALID_MODE;
	}
	ri.Printf(PRINT_ALL, " %d %d\n", glConfig.vidWidth, glConfig.vidHeight);

	if (fullscreen)
	{
		flags |= SDL_FULLSCREEN;
		glConfig.isFullscreen = qtrue;
	}
	else
	{
		if (noborder)
			flags |= SDL_NOFRAME;

		glConfig.isFullscreen = qfalse;
	}

	SDL_WM_SetCaption(WINDOW_CLASS_NAME, WINDOW_CLASS_NAME_MIN);
	SDL_ShowCursor(0);

	colorbits = r_colorbits->value;
	if ((!colorbits) || (colorbits >= 32))
		colorbits = 24;

	if (!r_depthbits->value)
		depthbits = 24;
	else
		depthbits = r_depthbits->value;

	stencilbits = r_stencilbits->value;

	for (i = 0; i < 16; i++)
	{
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i)
		{
			// one pass, reduce
			switch (i / 4)
			{
			case 2:
				if (colorbits == 24)
					colorbits = 16;
				break;
			case 1:
				if (depthbits == 24)
					depthbits = 16;
				else if (depthbits == 16)
					depthbits = 8;
			case 3:
				if (stencilbits == 24)
					stencilbits = 16;
				else if (stencilbits == 16)
					stencilbits = 8;
			}
		}

		tcolorbits   = colorbits;
		tdepthbits   = depthbits;
		tstencilbits = stencilbits;

		if ((i % 4) == 3) // reduce colorbits
		{
			if (tcolorbits == 24)
				tcolorbits = 16;
		}

		if ((i % 4) == 2) // reduce depthbits
		{
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1) // reduce stencilbits
		{
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		sdlcolorbits = 4;
		if (tcolorbits == 24)
			sdlcolorbits = 8;

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, sdlcolorbits);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, sdlcolorbits);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, sdlcolorbits);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, tdepthbits);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, tstencilbits);

		glConfig.stereoEnabled = qfalse;
		SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		if (!(SDLvidscreen = SDL_SetVideoMode(glConfig.vidWidth, glConfig.vidHeight, colorbits, flags)))
			continue;

		ri.Printf(PRINT_ALL, "Using %d/%d/%d Color bits, %d depth, %d stencil display.\n",
		          sdlcolorbits, sdlcolorbits, sdlcolorbits, tdepthbits, tstencilbits);

		glConfig.colorBits   = tcolorbits;
		glConfig.depthBits   = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		break;
	}

	if (!SDLvidscreen)
	{
		ri.Printf(PRINT_ALL, "Couldn't get a visual\n");
		return RSERR_INVALID_MODE;
	}
	return RSERR_OK;
}
	
qboolean GLimp_StartDriverAndSetMode(int mode, qboolean fullscreen, qboolean noborder)
{
	rserr_t err;

	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		char driverName[64];

		if (SDL_Init(SDL_INIT_VIDEO) == -1)
		{
			ri.Printf(PRINT_ALL, "SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n",
			          SDL_GetError());
			return qfalse;
		}

		SDL_VideoDriverName(driverName, sizeof(driverName) - 1);
		ri.Printf(PRINT_ALL, "SDL using driver \"%s\"\n", driverName);
		Cvar_Set("r_sdlDriver", driverName);

	}

	if (fullscreen && Cvar_VariableIntegerValue("in_nograb"))
	{
		ri.Printf(PRINT_ALL, "Fullscreen not allowed with in_nograb 1\n");
		ri.Cvar_Set("r_fullscreen", "0");
		r_fullscreen->modified = qfalse;
		fullscreen             = qfalse;
	}

	err = GLimp_SetMode(mode, fullscreen, noborder);

	switch (err)
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf(PRINT_ALL, "Fullscreen unavailable in mode: (%d)\n",mode);
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf(PRINT_ALL, "WARNING: could not set mode: (%d)\n", mode);
		return qfalse;
	default:
		ri.Printf(PRINT_ALL, "Set video mode: (%d)\n", mode);
		break;
	}

        if ( !qglGetString(GL_VENDOR) )
                return qfalse;

	return qtrue;
}

qboolean GLimp_HaveExtension( char *ext )
{
	char *ptr = Q_stristr(glConfig.extensions_string, ext);

	if (ptr == NULL)
	{
		return qfalse;
	}
	ptr += strlen(ext);
	return ((*ptr == ' ') || (*ptr == '\0'));
}

void GLimp_InitExtensions(void)
{
	ri.Printf(PRINT_ALL, "Initializing OpenGL extensions\n");

	glConfig.textureCompression = TC_NONE;

	if (glConfig.textureCompression == TC_NONE)
	{
		if (GLimp_HaveExtension("GL_S3_s3tc"))
		{
			if (r_ext_compressed_textures->value)
			{
				glConfig.textureCompression = TC_S3TC;
				ri.Printf(PRINT_ALL, "...using GL_S3_s3tc\n");
			}
			else
			{
				ri.Printf(PRINT_ALL, "...ignoring GL_S3_s3tc\n");
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "...GL_S3_s3tc not found\n");
		}
	}

	glConfig.textureEnvAddAvailable = qfalse;
	if (GLimp_HaveExtension("EXT_texture_env_add"))
	{
		if (r_ext_texture_env_add->integer)
		{
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_texture_env_add\n");
		}
		else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_texture_env_add not found\n");
	}

	qglMultiTexCoord2fARB     = NULL;
	qglActiveTextureARB       = NULL;
	qglClientActiveTextureARB = NULL;
	if (GLimp_HaveExtension("GL_ARB_multitexture"))
	{
		if (r_ext_multitexture->value)
		{
			qglMultiTexCoord2fARB     = SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
			qglActiveTextureARB       = SDL_GL_GetProcAddress("glActiveTextureARB");
			qglClientActiveTextureARB = SDL_GL_GetProcAddress("glClientActiveTextureARB");

			if (qglActiveTextureARB)
			{
				GLint glint = 0;
				qglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &glint);
				glConfig.maxActiveTextures = (int) glint;
				if (glConfig.maxActiveTextures > 1)
				{
					ri.Printf(PRINT_ALL, "...using GL_ARB_multitexture\n");
				}
				else
				{
					qglMultiTexCoord2fARB     = NULL;
					qglActiveTextureARB       = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf(PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n");
				}
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_multitexture\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_multitexture not found\n");
	}

	if (GLimp_HaveExtension("GL_EXT_compiled_vertex_array"))
	{
		if (r_ext_compiled_vertex_array->value)
		{
			ri.Printf(PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n");
			qglLockArraysEXT   = (void (APIENTRY *)(GLint, GLint))SDL_GL_GetProcAddress("glLockArraysEXT");
			qglUnlockArraysEXT = (void (APIENTRY *)(void))SDL_GL_GetProcAddress("glUnlockArraysEXT");
			if (!qglLockArraysEXT || !qglUnlockArraysEXT)
			{
				ri.Error(ERR_FATAL, "bad getprocaddress\n");
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n");
	}

}

void GLimp_Init( void ) {
	if (SDLvidscreen)
		return;

	if( !GLimp_StartDriverAndSetMode( r_mode->integer, r_fullscreen->integer , qfalse) )
		ri.Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem\n" );

	// Hardware gamma is not supported in recent Linux/SDL.
	glConfig.deviceSupportsGamma = qfalse;

	Q_strncpyz(glConfig.vendor_string, (char *) qglGetString(GL_VENDOR), sizeof(glConfig.vendor_string));
	Q_strncpyz(glConfig.renderer_string, (char *) qglGetString(GL_RENDERER), sizeof(glConfig.renderer_string));
	if (*glConfig.renderer_string && glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
	{
		glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;
	}
	Q_strncpyz(glConfig.version_string, (char *) qglGetString(GL_VERSION), sizeof(glConfig.version_string));
	glConfig.extensions_string = (char *)qglGetString(GL_EXTENSIONS);

	GLimp_InitExtensions();
	glConfig.stereoEnabled = qfalse;
	IN_Init();
	return;
}

/*
Responsible for doing a swapbuffers and possibly for other stuff
*/
void GLimp_EndFrame( void ) {
	// don't flip if drawing to front buffer
	if (Q_stricmp(r_drawBuffer->string, "GL_FRONT") != 0)
		SDL_GL_SwapBuffers();
}

// Obsolete stub
void Sys_SendKeyEvents( void ) {
}

/*
===============
SMP
===============
*/
char   GLthreadname[16]			= "rtcwSMP";
static SDL_mutex  *smpMutex             = NULL;
static SDL_cond   *renderCommandsEvent  = NULL;
static SDL_cond   *renderCompletedEvent = NULL;
static void  (*glimpRenderThread)(void) = NULL;
static SDL_Thread *renderThread         = NULL;
static volatile void *smpData           = NULL;

/*
===============
GLimp_ShutdownRenderThread
===============
*/
void GLimp_ShutdownRenderThread(void)
{
	if (smpMutex != NULL)
	{
		SDL_DestroyMutex(smpMutex);
		smpMutex = NULL;
	}
	if (renderCommandsEvent != NULL)
	{
		SDL_DestroyCond(renderCommandsEvent);
		renderCommandsEvent = NULL;
	}
	if (renderCompletedEvent != NULL)
	{
		SDL_DestroyCond(renderCompletedEvent);
		renderCompletedEvent = NULL;
	}
}

/*
===============
GLimp_RenderThreadWrapper
===============
*/
int GLimp_RenderThreadWrapper(void *arg)
{
	Com_Printf("SMP: Render thread starting.\n");
	glimpRenderThread();

	SDL_LockMutex(smpMutex);
	{
		smpData = NULL;
		renderThread = NULL;
		SDL_CondSignal(renderCompletedEvent);
	}
	SDL_UnlockMutex(smpMutex);

	Com_Printf("SMP: Render thread ended.\n");
	return 0;
}

/*
===============
GLimp_SpawnRenderThread
===============
*/
qboolean GLimp_SpawnRenderThread(void (*function)(void))
{

	if (renderThread == (void *)0xdead)
	{
		Com_Printf("SMP: Not safe restarting thread.\n");
		renderThread = NULL;
		return qfalse;
	}
	if (renderThread != NULL)
	{
		Com_Printf("SMP: Render thread still Running.\n");
		return qtrue;
	}
	smpData = (void *)0xdead;

	smpMutex = SDL_CreateMutex();
	if (!smpMutex) 
	{
		Com_Printf("SMP: Mutex creation failed: %s\n", SDL_GetError());
		GLimp_ShutdownRenderThread();
		return qfalse;
	}

	renderCommandsEvent = SDL_CreateCond();
	if (renderCommandsEvent == NULL)
	{
		Com_Printf("SMP: CommandEvent creation failed: %s\n", SDL_GetError());
		GLimp_ShutdownRenderThread();
		return qfalse;
	}

	renderCompletedEvent = SDL_CreateCond();
	if (renderCompletedEvent == NULL)
	{
		Com_Printf("SMP: CompletedEvent creation failed: %s\n", SDL_GetError());
		GLimp_ShutdownRenderThread();
		return qfalse;
	}

	glimpRenderThread = function;
	renderThread      = SDL_CreateThread(GLimp_RenderThreadWrapper, GLthreadname);
	if (renderThread == NULL)
	{
		ri.Printf(PRINT_ALL, "SDL: CreateThread() returned %s", SDL_GetError());
		GLimp_ShutdownRenderThread();
		return qfalse;
	}

	SDL_LockMutex(smpMutex);
		while ( smpData )
                         SDL_CondWait(renderCompletedEvent, smpMutex);
	SDL_UnlockMutex(smpMutex);

	return qtrue;
}

/*
===============
GLimp_RendererSleep
===============
*/
void *GLimp_RendererSleep(void)
{
	void *data = NULL;

	SDL_LockMutex(smpMutex);
	{
		smpData      = NULL;
		// after this, the front end can exit GLimp_FrontEndSleep
		SDL_CondSignal(renderCompletedEvent);

		while (!smpData)
			SDL_CondWait(renderCommandsEvent, smpMutex);

		data = (void *)smpData;
		if (data == (void *)0xdead )
		{
			renderThread = (void *)0xdead;
			data = NULL;
		}
	}
	SDL_UnlockMutex(smpMutex);

	return data;
}

/*
===============
GLimp_FrontEndSleep
===============
*/
void GLimp_FrontEndSleep(void)
{
	SDL_LockMutex(smpMutex);
	{
		while (smpData)
			SDL_CondWait(renderCompletedEvent, smpMutex);
	}
	SDL_UnlockMutex(smpMutex);
}

/*
===============
GLimp_WakeRenderer
===============
*/
void GLimp_WakeRenderer(void *data)
{
	SDL_LockMutex(smpMutex);
	{
		// Should be NULL if we called GLimp_FrontEndSleep()
		while (smpData)
			SDL_CondWait(renderCompletedEvent, smpMutex);

		smpData = data;
		SDL_CondSignal(renderCommandsEvent);
	}
	SDL_UnlockMutex(smpMutex);
}

