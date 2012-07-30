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
** linux_joystick.c
**
** This file contains ALL Linux specific stuff having to do with the
** Joystick input.  When a port is being made the following functions
** must be implemented by the port:
**
** Authors: mkv, bk
**
*/

#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>  // bk001204

#include "../client/client.h"
#include "linux_local.h"

// TTimo moved joystick.h include here, conflicts in client.h with wkey_t
#include <linux/joystick.h>

/* We translate axes movement into mouse moves. */

/* Our file descriptor for the joystick device. */
static int joy_fd = -1;
static int joy_lastx = 0;
static int joy_lasty = 0;
static int joy_lastz = 0;

/**********************************************/
/* Joystick routines.                         */
/**********************************************/
// bk001130 - from cvs1.17 (mkv), removed from linux_glimp.c
void IN_StartupJoystick( void ) {
	int i = 0;
	joy_fd = -1;

	for ( i = 0; i < 4; i++ ) {
		char filename[PATH_MAX];

		snprintf( filename, PATH_MAX, "/dev/input/js%d", i );

		joy_fd = open( filename, O_RDONLY | O_NONBLOCK );

		if ( joy_fd != -1 ) {
			struct js_event event;
			char axes = 0;
			char buttons = 0;
			char name[128];
			int n = -1;

			Com_Printf( "Joystick %s found\n", filename );

			/* Get rid of initialization messages. */
			do {
				n = read( joy_fd, &event, sizeof( event ) );

				if ( n == -1 ) {
					break;
				}

			} while ( ( event.type & JS_EVENT_INIT ) );

			/* Get joystick statistics. */
			ioctl( joy_fd, JSIOCGAXES, &axes );
			ioctl( joy_fd, JSIOCGBUTTONS, &buttons );

			if ( ioctl( joy_fd, JSIOCGNAME( sizeof( name ) ), name ) < 0 ) {
				strncpy( name, "Unknown", sizeof( name ) );
			}

			Com_Printf( "Name:    %s\n", name );
			Com_Printf( "Axes:    %d\n", axes );
			Com_Printf( "Buttons: %d\n", buttons );

			/* Our work here is done. */
			return;
		}

	}

	/* No soup for you. */
	if ( joy_fd == -1 ) {
		Com_Printf( "No joystick found.\n" );
		return;
	}

}
#define SIMMOUSEBTNS 3
void IN_JoyMove( void ) {
	unsigned int axes = 0;
	int i = 0;

	if ( joy_fd == -1 ) {
		return;
	}

	/* Empty the queue, dispatching button presses immediately
	   * and updating the instantaneous state for the axes.
	   */
	int t = Sys_Milliseconds();
	do {
		int n = -1;
		struct js_event event;

		n = read( joy_fd, &event, sizeof( event ) );

		if ( n == -1 ) {
			Sys_QueEvent( t, SE_MOUSE, joy_lastx, joy_lasty, 0, NULL );
			break;
		}

		if ( event.type & JS_EVENT_BUTTON ) {
			if ( event.number < SIMMOUSEBTNS )
				Sys_QueEvent( 0, SE_KEY, K_MOUSE1 + event.number, event.value, 0, NULL );
			else
				Sys_QueEvent( 0, SE_KEY, K_JOY1 + event.number - SIMMOUSEBTNS, event.value, 0, NULL );
		} else if ( event.type & JS_EVENT_AXIS ) {
			axes = event.value  >> 9;
			    if (event.number == 0)
				joy_lastx = axes;
			    else if (event.number == 1)
				joy_lasty = axes;
			    else if (event.number == 2)
				joy_lastz = axes;
		}//TODO Find use for Third Axis : should set speed or field of view.
	} while ( 1 );


	/* Translate our instantaneous state to bits. */
/*	for ( i = 0; i < 16; i++ ) {
		float f = ( (float) axes_state[i] ) / 32767.0f;

		if ( f < -joy_threshold->value ) {
			axes |= ( 1 << ( i * 2 ) );
		} else if ( f > joy_threshold->value ) {
			axes |= ( 1 << ( ( i * 2 ) + 1 ) );
		}

	}
*/
	/* Time to update axes state based on old vs. new. */
/*	for ( i = 0; i < 16; i++ ) {

		if ( ( axes & ( 1 << i ) ) && !( old_axes & ( 1 << i ) ) ) {
			Sys_QueEvent( 0, SE_KEY, joy_keys[i], qtrue, 0, NULL );
		}

		if ( !( axes & ( 1 << i ) ) && ( old_axes & ( 1 << i ) ) ) {
			Sys_QueEvent( 0, SE_KEY, joy_keys[i], qfalse, 0, NULL );
		}
	}
*/
	/* Save for future generations. */
//	old_axes = axes;
}


