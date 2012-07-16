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

static qboolean mouse_avail;
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

qboolean dgamouse = qfalse;
static int win_x, win_y;
static int mouse_accel_numerator;
static int mouse_accel_denominator;
static int mouse_threshold;

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

/*****************************************************************************
** NOTE TTimo the keyboard handling is done with KeySyms
**   that means relying on the keyboard mapping provided by X
******************************************************************************/
/*
static char *XLateKey( XKeyEvent *ev, int *key ) {
}
	static char buf[64];
	KeySym keysym;
	int XLookupRet;

	*key = 0;
	XLookupRet = XLookupString( ev, buf, sizeof buf, &keysym, 0 );
#ifdef KBD_DBG
	ri.Printf( PRINT_ALL, "XLookupString ret: %d buf: %s keysym: %x\n", XLookupRet, buf, keysym );
#endif
	switch ( keysym )
	{
	case XK_KP_Page_Up:
	case XK_KP_9:  *key = K_KP_PGUP; break;
	case XK_Page_Up:   *key = K_PGUP; break;

	case XK_KP_Page_Down:
	case XK_KP_3: *key = K_KP_PGDN; break;
	case XK_Page_Down:   *key = K_PGDN; break;

	case XK_KP_Home: *key = K_KP_HOME; break;
	case XK_KP_7: *key = K_KP_HOME; break;
	case XK_Home:  *key = K_HOME; break;

	case XK_KP_End:
	case XK_KP_1:   *key = K_KP_END; break;
	case XK_End:   *key = K_END; break;

	case XK_KP_Left: *key = K_KP_LEFTARROW; break;
	case XK_KP_4: *key = K_KP_LEFTARROW; break;
	case XK_Left:  *key = K_LEFTARROW; break;

	case XK_KP_Right: *key = K_KP_RIGHTARROW; break;
	case XK_KP_6: *key = K_KP_RIGHTARROW; break;
	case XK_Right:  *key = K_RIGHTARROW;    break;

	case XK_KP_Down:
	case XK_KP_2:    *key = K_KP_DOWNARROW; break;
	case XK_Down:  *key = K_DOWNARROW; break;

	case XK_KP_Up:
	case XK_KP_8:    *key = K_KP_UPARROW; break;
	case XK_Up:    *key = K_UPARROW;   break;

	case XK_Escape: *key = K_ESCAPE;    break;
	case XK_KP_Enter: *key = K_KP_ENTER;  break;
	case XK_Return: *key = K_ENTER;    break;
	case XK_Tab:    *key = K_TAB;      break;

	case XK_F1:    *key = K_F1;       break;
	case XK_F2:    *key = K_F2;       break;
	case XK_F3:    *key = K_F3;       break;
	case XK_F4:    *key = K_F4;       break;
	case XK_F5:    *key = K_F5;       break;
	case XK_F6:    *key = K_F6;       break;
	case XK_F7:    *key = K_F7;       break;
	case XK_F8:    *key = K_F8;       break;
	case XK_F9:    *key = K_F9;       break;
	case XK_F10:    *key = K_F10;      break;
	case XK_F11:    *key = K_F11;      break;
	case XK_F12:    *key = K_F12;      break;
		//case XK_BackSpace: *key = 8; break; // ctrl-h
	case XK_BackSpace: *key = K_BACKSPACE; break; // ctrl-h
	case XK_KP_Delete:
	case XK_KP_Decimal: *key = K_KP_DEL; break;
	case XK_Delete: *key = K_DEL; break;

	case XK_Pause:  *key = K_PAUSE;    break;

	case XK_Shift_L:
	case XK_Shift_R:  *key = K_SHIFT;   break;

	case XK_Execute:
	case XK_Control_L:
	case XK_Control_R:  *key = K_CTRL;  break;

	case XK_Alt_L:
	case XK_Meta_L:
	case XK_Alt_R:
	case XK_Meta_R: *key = K_ALT;     break;

	case XK_KP_Begin: *key = K_KP_5;  break;

	case XK_Insert:   *key = K_INS; break;
	case XK_KP_Insert:
	case XK_KP_0: *key = K_KP_INS; break;

	case XK_KP_Multiply: *key = '*'; break;
	case XK_KP_Add:  *key = K_KP_PLUS; break;
	case XK_KP_Subtract: *key = K_KP_MINUS; break;
	case XK_KP_Divide: *key = K_KP_SLASH; break;

	case XK_exclam: *key = '1'; break;
	case XK_at: *key = '2'; break;
	case XK_numbersign: *key = '3'; break;
	case XK_dollar: *key = '4'; break;
	case XK_percent: *key = '5'; break;
	case XK_asciicircum: *key = '6'; break;
	case XK_ampersand: *key = '7'; break;
	case XK_asterisk: *key = '8'; break;
	case XK_parenleft: *key = '9'; break;
	case XK_parenright: *key = '0'; break;
		// weird french keyboards ..
	case XK_twosuperior: *key = '~'; break;

	default:
		if ( XLookupRet == 0 ) {
			if ( com_developer->value ) {
				ri.Printf( PRINT_ALL, "Warning: XLookupString failed on KeySym %d\n", keysym );
			}
			return NULL;
		} else
		{
			// XK_* tests failed, but XLookupString got a buffer, so let's try it
			*key = *(unsigned char *)buf;
			if ( *key >= 'A' && *key <= 'Z' ) {
				*key = *key - 'A' + 'a';
			}
			// if ctrl is pressed, the keys are not between 'A' and 'Z', for instance ctrl-z == 26 ^Z ^C etc.
			// see show_bug.cgi?id=19
			else if ( *key >= 1 && *key <= 26 ) {
				*key = *key + 'a' - 1;
			}
		}
		break;
	}

	return buf;
}
*/
/* makes a null cursor */
/*
static Cursor CreateNullCursor( Display *display, Window root ) {
}
	Pixmap cursormask;
	XGCValues xgc;
	GC gc;
	XColor dummycolour;
	Cursor cursor;

	cursormask = XCreatePixmap( display, root, 1, 1, 1 );
	xgc.function = GXclear;
	gc =  XCreateGC( display, cursormask, GCFunction, &xgc );
	XFillRectangle( display, cursormask, gc, 0, 0, 1, 1 );
	dummycolour.pixel = 0;
	dummycolour.red = 0;
	dummycolour.flags = 04;
	cursor = XCreatePixmapCursor( display, cursormask, cursormask,
								  &dummycolour,&dummycolour, 0,0 );
	XFreePixmap( display,cursormask );
	XFreeGC( display,gc );
	return cursor;
}
*/
static void install_grabs( void ) {
}/*
	// inviso cursor
	XWarpPointer( dpy, None, win,
				  0, 0, 0, 0,
				  glConfig.vidWidth / 2, glConfig.vidHeight / 2 );
	XSync( dpy, False );
	XDefineCursor( dpy, win, CreateNullCursor( dpy, win ) );
	XGrabPointer( dpy, win,
				  False,
				  MOUSE_MASK,
				  GrabModeAsync, GrabModeAsync,
				  win,
				  None,
				  CurrentTime );

	XGetPointerControl( dpy, &mouse_accel_numerator, &mouse_accel_denominator,
						&mouse_threshold );

	XChangePointerControl( dpy, True, True, 1, 1, 0 );
	XSync( dpy, False );
	mouseResetTime = Sys_Milliseconds();

#if 0 // disable XF86DGA
	if ( in_dgamouse->value ) {
		int MajorVersion, MinorVersion;

		if ( !XF86DGAQueryVersion( dpy, &MajorVersion, &MinorVersion ) ) {
			// unable to query, probalby not supported
			ri.Printf( PRINT_ALL, "Failed to detect XF86DGA Mouse\n" );
			ri.Cvar_Set( "in_dgamouse", "0" );
		} else
		{
			dgamouse = qtrue;
			XF86DGADirectVideo( dpy, DefaultScreen( dpy ), XF86DGADirectMouse );
			XWarpPointer( dpy, None, win, 0, 0, 0, 0, 0, 0 );
		}
	} else
	{
		mwx = glConfig.vidWidth / 2;
		mwy = glConfig.vidHeight / 2;
		mx = my = 0;
	}
#else 
	ri.Printf( PRINT_ALL, "XF86DGA Mouse Disabled\n" );
        ri.Cvar_Set( "in_dgamouse", "0" );
	mwx = glConfig.vidWidth / 2;
        mwy = glConfig.vidHeight / 2;
	mx = my = 0;

#endif  // disable XF86DGA

	XGrabKeyboard( dpy, win,
				   False,
				   GrabModeAsync, GrabModeAsync,
				   CurrentTime );
	XSync( dpy, False );
}
*/
static void uninstall_grabs( void ) {
}/*
	if ( dgamouse ) { // DGA is disabled ?
		dgamouse = qfalse;
//		XF86DGADirectVideo( dpy, DefaultScreen( dpy ), 0 );
	}
	XChangePointerControl( dpy, qtrue, qtrue, mouse_accel_numerator,
						   mouse_accel_denominator, mouse_threshold );

	XUngrabPointer( dpy, CurrentTime );
	XUngrabKeyboard( dpy, CurrentTime );
	XWarpPointer( dpy, None, win,
				  0, 0, 0, 0,
				  glConfig.vidWidth / 2, glConfig.vidHeight / 2 );
	// inviso cursor
	XUndefineCursor( dpy, win );
}
*/
static qboolean X11_PendingInput( void ) {
}/*
	assert( dpy != NULL );
	XFlush( dpy );
	if ( XEventsQueued( dpy, QueuedAlready ) ) {
		return qtrue;
	}
	//else
	{
		static struct timeval zero_time;
		int x11_fd;
		fd_set fdset;

		x11_fd = ConnectionNumber( dpy );
		FD_ZERO( &fdset );
		FD_SET( x11_fd, &fdset );
		if ( select( x11_fd + 1, &fdset, NULL, NULL, &zero_time ) == 1 ) {
			return( XPending( dpy ) );
		}
	}
	return qfalse;
}
*/
/*
static qboolean repeated_press( XEvent *event ) {
}
	XEvent peekevent;
	qboolean repeated = qfalse;

	assert( dpy != NULL );
	if ( X11_PendingInput() ) {
		XPeekEvent( dpy, &peekevent );

		if ( ( peekevent.type == KeyPress ) &&
			 ( peekevent.xkey.keycode == event->xkey.keycode ) &&
			 ( peekevent.xkey.time == event->xkey.time ) ) {
			repeated = qtrue;
			XNextEvent( dpy, &peekevent ); // skip event.
		}
	}
	return( repeated );
}
*/
static void HandleEvents( void ) {
}/*
	int b;
	int key;
	XEvent event;
	qboolean dowarp = qfalse;
	char *p;
	int dx, dy;
	int t;

	if ( !dpy ) {
		return;
	}

	while ( XPending( dpy ) )
	{
		XNextEvent( dpy, &event );
		switch ( event.type )
		{
		case KeyPress:
			p = XLateKey( &event.xkey, &key );
			if ( key ) {
				Sys_QueEvent( 0, SE_KEY, key, qtrue, 0, NULL );
			}
			if ( p ) {
				while ( *p )
				{
					Sys_QueEvent( 0, SE_CHAR, *p++, 0, 0, NULL );
				}
			}
			break;

		case KeyRelease:
			if ( cls.keyCatchers == 0 ) { // FIXME: KEYCATCH_NONE
				if ( repeated_press( &event ) == qtrue ) {
					continue;
				}
			}
			XLateKey( &event.xkey, &key );

			Sys_QueEvent( 0, SE_KEY, key, qfalse, 0, NULL );
			break;

		case MotionNotify:
			if ( mouse_active ) {
				if ( dgamouse ) {
					if ( abs( event.xmotion.x_root ) > 1 ) {
						mx += event.xmotion.x_root * 2;
					} else {
						mx += event.xmotion.x_root;
					}
					if ( abs( event.xmotion.y_root ) > 1 ) {
						my += event.xmotion.y_root * 2;
					} else {
						my += event.xmotion.y_root;
					}
					t = Sys_Milliseconds();
					if ( t - mouseResetTime > MOUSE_RESET_DELAY ) {
						Sys_QueEvent( t, SE_MOUSE, mx, my, 0, NULL );
					}
					mx = my = 0;
				} else
				{
					// If it's a center motion, we've just returned from our warp
					if ( event.xmotion.x == glConfig.vidWidth / 2 &&
						 event.xmotion.y == glConfig.vidHeight / 2 ) {
						mwx = glConfig.vidWidth / 2;
						mwy = glConfig.vidHeight / 2;
						t = Sys_Milliseconds();
						if ( t - mouseResetTime > MOUSE_RESET_DELAY ) {
							Sys_QueEvent( t, SE_MOUSE, mx, my, 0, NULL );
						}
						mx = my = 0;
						break;
					}

					dx = ( (int)event.xmotion.x - mwx );
					dy = ( (int)event.xmotion.y - mwy );
					if ( abs( dx ) > 1 ) {
						mx += dx * 2;
					} else {
						mx += dx;
					}
					if ( abs( dy ) > 1 ) {
						my += dy * 2;
					} else {
						my += dy;
					}

					mwx = event.xmotion.x;
					mwy = event.xmotion.y;
					dowarp = qtrue;
				}
			}
			break;

		case ButtonPress:
			if ( event.xbutton.button == 4 ) {
				Sys_QueEvent( 0, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
			} else if ( event.xbutton.button == 5 ) {
				Sys_QueEvent( 0, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
			} else
			{
				// NOTE TTimo there seems to be a weird mapping for K_MOUSE1 K_MOUSE2 K_MOUSE3 ..
				b = -1;
				if ( event.xbutton.button == 1 ) {
					b = 0; // K_MOUSE1
				} else if ( event.xbutton.button == 2 ) {
					b = 2; // K_MOUSE3
				} else if ( event.xbutton.button == 3 ) {
					b = 1; // K_MOUSE2
				} else if ( event.xbutton.button == 6 ) {
					b = 3; // K_MOUSE4
				} else if ( event.xbutton.button == 7 ) {
					b = 4; // K_MOUSE5
				}
				;

				Sys_QueEvent( 0, SE_KEY, K_MOUSE1 + b, qtrue, 0, NULL );
			}
			break;

		case ButtonRelease:
			if ( event.xbutton.button == 4 ) {
				Sys_QueEvent( 0, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
			} else if ( event.xbutton.button == 5 ) {
				Sys_QueEvent( 0, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
			} else
			{
				b = -1;
				if ( event.xbutton.button == 1 ) {
					b = 0;
				} else if ( event.xbutton.button == 2 ) {
					b = 2;
				} else if ( event.xbutton.button == 3 ) {
					b = 1;
				} else if ( event.xbutton.button == 6 ) {
					b = 3; // K_MOUSE4
				} else if ( event.xbutton.button == 7 ) {
					b = 4; // K_MOUSE5
				}
				;
				Sys_QueEvent( 0, SE_KEY, K_MOUSE1 + b, qfalse, 0, NULL );
			}
			break;

		case CreateNotify:
			win_x = event.xcreatewindow.x;
			win_y = event.xcreatewindow.y;
			break;

		case ConfigureNotify:
			win_x = event.xconfigure.x;
			win_y = event.xconfigure.y;
			break;
		}
	}

	if ( dowarp ) {
		XWarpPointer( dpy,None,win,0,0,0,0,
					  ( glConfig.vidWidth / 2 ),( glConfig.vidHeight / 2 ) );
	}
}
*/
void IN_ActivateMouse( void ) {
/*
	if ( !mouse_avail || !dpy || !win ) {
		return;
	}
*/
	if ( !mouse_active ) {
		install_grabs();
		mouse_active = qtrue;
	}
}

void IN_DeactivateMouse( void ) {
/*
	if ( !mouse_avail || !dpy || !win ) {
		return;
	}
*/
	if ( mouse_active ) {
		uninstall_grabs();
		mouse_active = qfalse;
	}
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
	IN_DeactivateMouse();

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
	EGL_RED_SIZE,       5,
	EGL_GREEN_SIZE,     6,
	EGL_BLUE_SIZE,      5,
	EGL_ALPHA_SIZE,     0,
	EGL_DEPTH_SIZE,	   16,
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
	glConfig.maxTextureSize = 512; //upto 2048 with unlimited ram
	glConfig.maxActiveTextures=8; //Only 2 used 
	return;
}

/*
Responsible for doing a swapbuffers and possibly for other stuff
Only clear buffers once per frame
*/
void GLimp_EndFrame( void ) {
	eglSwapBuffers(g_EGLDisplay, g_EGLWindowSurface);
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

/* Mouse/Joystick */
void IN_Init( void ) {
	in_mouse = Cvar_Get( "in_mouse", "0", CVAR_ARCHIVE );
	in_dgamouse = 0;
	in_joystick = Cvar_Get( "in_joystick", "1", CVAR_ARCHIVE | CVAR_LATCH );
	in_joystickDebug = Cvar_Get( "in_debugjoystick", "0", CVAR_TEMP );
	joy_threshold = Cvar_Get( "joy_threshold", "0.15", CVAR_ARCHIVE ); // FIXME: in_joythreshold
	if ( in_mouse->value ) {
		mouse_avail = qtrue;
	} else {
		mouse_avail = qfalse;
	}
	IN_StartupJoystick();
}

void IN_Shutdown( void ) {
	mouse_avail = qfalse;
}

void IN_Frame( void ) {

/*	IN_JoyMove(); - moved to emulate mouse */
	IN_ActivateMouse();
}

void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
}

void IN_Activate( void ) {
}

void Sys_SendKeyEvents( void ) {
        IN_JoyMove();
}

