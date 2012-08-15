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
*/

/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
** GLimp_SetGamma
**
*/

#include <bcm_host.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include <dlfcn.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "linux_local.h"

#include "unix_glw.h"

#define WINDOW_CLASS_NAME   "Return to Castle Wolfenstein"

typedef enum
{
	RSERR_OK,
	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,
	RSERR_UNKNOWN
} rserr_t;

glwstate_t glw_state;

/*
static qboolean mouse_active;
static int mwx, mwy;
static int mx = 0, my = 0;
static int mouseResetTime = 0;

#define KEY_MASK ( KeyPressMask | KeyReleaseMask )
#define MOUSE_MASK ( ButtonPressMask | ButtonReleaseMask | \
                       PointerMotionMask | ButtonMotionMask )
#define X_MASK ( KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )
#define MOUSE_RESET_DELAY 50
static cvar_t *in_mouse;
static cvar_t *in_dgamouse;

static cvar_t   *in_joystick;
static cvar_t   *in_joystickDebug;
static cvar_t   *joy_threshold;

static int win_x, win_y;
static int mouse_accel_numerator;
static int mouse_accel_denominator;
static int mouse_threshold;
*/

static const char *Q_stristr( const char *s, const char *find ) {
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
static qboolean X11_PendingInput( void ) {
}
static void HandleEvents( void ) {
}
static qboolean signalcaught = qfalse;;

void Sys_Exit( int );

static void signal_handler( int sig ) {
	if ( signalcaught ) {
		printf( "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n", sig );
		Sys_Exit( 1 );
	}
	signalcaught = qtrue;
	printf( "Received signal %d, exiting...\n", sig );
	GLimp_Shutdown(); // bk010104 - shouldn't this be CL_Shutdown
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

/* GLimp_Shutdown */
void GLimp_Shutdown( void ) {
	IN_Shutdown();
	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
        bcm_host_deinit();
}

/* GLimp_LogComment */
void GLimp_LogComment( char *comment ) {
}

/* GLW_StartDriverAndSetMode */
static EGLDisplay   g_EGLDisplay;
static EGLConfig    g_EGLConfig;
static EGLContext   g_EGLContext;
static EGLSurface   g_EGLWindowSurface;
static qboolean GLimp_StartDriver()
{
   /* TODO cleanup on failure... */
	const EGLint s_configAttribs[] =
	{
	EGL_RED_SIZE,       8,
	EGL_GREEN_SIZE,     8,
	EGL_BLUE_SIZE,      8,
	EGL_ALPHA_SIZE,     0,
	EGL_DEPTH_SIZE,	   24,
	EGL_STENCIL_SIZE,   0,
	EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
	EGL_SAMPLE_BUFFERS, 1,
	EGL_NONE
	};

   EGLint numConfigs;
   EGLint majorVersion;
   EGLint minorVersion;

   g_EGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (g_EGLDisplay == EGL_NO_DISPLAY) {
      ri.Printf(PRINT_ALL, "eglGetDisplay() failed\n");
      return qfalse;
   }
   if (!eglInitialize(g_EGLDisplay, &majorVersion, &minorVersion)) {
      ri.Printf(PRINT_ALL, "eglInitialize() failed\n");
      return qfalse;
   }
   if (!eglSaneChooseConfigBRCM(g_EGLDisplay, s_configAttribs, &g_EGLConfig, 1, &numConfigs)) {
      ri.Printf(PRINT_ALL, "eglSaneChooseConfigBRCM() failed\n");
      return qfalse;
   }
   if (numConfigs == 0) {
      ri.Printf(PRINT_ALL, "Couldn't find suitable config!\n");
      return qfalse;
   }

   {
      EGLint r, g, b, a, depth, stencil, samples, sample_buffers;
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_RED_SIZE, &r);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_GREEN_SIZE, &g);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_BLUE_SIZE, &b);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_ALPHA_SIZE, &a);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_DEPTH_SIZE, &depth);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_STENCIL_SIZE, &stencil);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_SAMPLES, &samples);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_SAMPLE_BUFFERS, &sample_buffers);
      ri.Printf(PRINT_ALL, "EGL config %d: r=%d,g=%d,b=%d,a=%d, "
         "depth=%d,stencil=%d, samples=%d,sample_buffers=%d\n",
         (int)g_EGLConfig, r, g, b, a, depth, stencil, samples, sample_buffers);
   }

   EGLBoolean ignore_result = eglBindAPI(EGL_OPENGL_ES_API);

   g_EGLContext = eglCreateContext(g_EGLDisplay, g_EGLConfig, NULL, NULL);
   if (g_EGLContext == EGL_NO_CONTEXT) {
      ri.Printf(PRINT_ALL, "eglCreateContext() failed\n");
      return qfalse;
   }

   g_EGLWindowSurface = eglCreateWindowSurface(g_EGLDisplay, g_EGLConfig, 0, NULL);
   if (g_EGLWindowSurface == EGL_NO_SURFACE) {
      ri.Printf(PRINT_ALL, "eglCreateWindowSurface() failed\n");
      return qfalse;
   }
   eglMakeCurrent(g_EGLDisplay, g_EGLWindowSurface, g_EGLWindowSurface, g_EGLContext);
   {
      EGLint width, height, color, depth, stencil;
      eglQuerySurface(g_EGLDisplay, g_EGLWindowSurface, EGL_WIDTH, &width);
      eglQuerySurface(g_EGLDisplay, g_EGLWindowSurface, EGL_HEIGHT, &height);
      ri.Printf(PRINT_ALL, "Window size: %dx%d\n", width, height);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_BUFFER_SIZE, &color);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_DEPTH_SIZE, &depth);
      eglGetConfigAttrib(g_EGLDisplay, g_EGLConfig, EGL_STENCIL_SIZE, &stencil);
      glConfig.vidWidth = width;
      glConfig.vidHeight = height;
      glConfig.colorBits = color;
      glConfig.depthBits = depth;
      glConfig.stencilBits = stencil;
      }

	return qtrue;
}


/*
** This routine is responsible for initializing the OS specific portion sof OpenGL.
*/
void GLimp_Init( void ) {
	bcm_host_init();

	if( !GLimp_StartDriver() )
		ri.Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem\n" );

	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;
	glConfig.deviceSupportsGamma = qfalse;
	glConfig.anisotropicAvailable = qfalse;
	glConfig.stereoEnabled = qfalse;
	glConfig.maxTextureSize = 1024; //upto 2048 with unlimited ram
	glConfig.maxActiveTextures=8; //No more than 8 
	return;
}

/*
Responsible for doing a swapbuffers and possibly for other stuff
*/
void GLimp_EndFrame( void ) {
	eglSwapBuffers(g_EGLDisplay, g_EGLWindowSurface);
/*	IN_JoyMove(); // mouse emulates joystick: move on every frame */
}

/* SINGLE CPU*/
void GLimp_RenderThreadWrapper( void *stub ) {}
qboolean GLimp_SpawnRenderThread( void ( *function )( void ) ) {
	return qfalse;
}
void *GLimp_RendererSleep( void ) {
	return NULL;
}
void GLimp_FrontEndSleep( void ) {}
void GLimp_WakeRenderer( void *data ) {}
#if 0 //pre-SDL code hooks
/* Mouse/Joystick */
void IN_Init( void ) {

	nonblock(1);//Q3 tty keyboard on

	IN_StartupJoystick();
}

void IN_Shutdown( void ) {

	nonblock(0);//Q3 tty keyboard off

// close joystick !
}

void IN_Frame( void ) {

/*	IN_MouseMove(); */
}

void IN_Activate( void ) {
}
#endif
void Sys_SendKeyEvents( void ) {
//	IN_ProcessEvents( ); //now part of SDL_input.c
}

void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
}


