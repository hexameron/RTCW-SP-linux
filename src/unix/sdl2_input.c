/*
 * Wolfenstein: Enemy Territory GPL Source Code
 * Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
 *
 * ET: Legacy
 * Copyright (C) 2012 Jan Simek <mail@etlegacy.com>
 *
 * This file is part of ET: Legacy - http://www.etlegacy.com
 *
 * ET: Legacy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ET: Legacy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ET: Legacy. If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, Wolfenstein: Enemy Territory GPL Source Code is also
 * subject to certain additional terms. You should have received a copy
 * of these additional terms immediately following the terms and conditions
 * of the GNU General Public License which accompanied the source code.
 * If not, please request a copy in writing from id Software at the address below.
 *
 * id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
 *
 * @file sdl_input.c
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_video.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../client/client.h"
#include "./linux_local.h"

static cvar_t *in_nograb;
static cvar_t *in_keyboardDebug = NULL;

static cvar_t   *in_mouse        = NULL;
static qboolean mouseAvailable   = qfalse;
static qboolean mouseActive      = qfalse;
//static qboolean keyRepeatEnabled = qfalse;
extern SDL_Window *SDLvidscreen;

static SDL_Joystick *stick                = NULL;
static cvar_t       *in_joystick          = NULL;
static cvar_t       *in_joystickDebug     = NULL;
static cvar_t       *in_joystickThreshold = NULL;
static cvar_t       *in_joystickNo        = NULL;
static cvar_t       *in_joystickUseAnalog = NULL;

static int vidRestartTime = 0;

#define CTRL(a) ((a) - 'a' + 1)

/*
 * @brief Prints keyboard identifiers in the console
 */
static void IN_PrintKey(const SDL_Keysym *keysym, keyNum_t key, qboolean down)
{
	if (down)
	{
		Com_Printf("+ ");
	}
	else
	{
		Com_Printf("  ");
	}

	Com_Printf("0x%02x \"%s\"", keysym->scancode,
	           SDL_GetKeyName(keysym->sym));

	if (keysym->mod & KMOD_LSHIFT)
	{
		Com_Printf(" KMOD_LSHIFT");
	}
	if (keysym->mod & KMOD_RSHIFT)
	{
		Com_Printf(" KMOD_RSHIFT");
	}
	if (keysym->mod & KMOD_LCTRL)
	{
		Com_Printf(" KMOD_LCTRL");
	}
	if (keysym->mod & KMOD_RCTRL)
	{
		Com_Printf(" KMOD_RCTRL");
	}
	if (keysym->mod & KMOD_LALT)
	{
		Com_Printf(" KMOD_LALT");
	}
	if (keysym->mod & KMOD_RALT)
	{
		Com_Printf(" KMOD_RALT");
	}
	if (keysym->mod & KMOD_NUM)
	{
		Com_Printf(" KMOD_NUM");
	}
	if (keysym->mod & KMOD_CAPS)
	{
		Com_Printf(" KMOD_CAPS");
	}
	if (keysym->mod & KMOD_MODE)
	{
		Com_Printf(" KMOD_MODE");
	}
	if (keysym->mod & KMOD_RESERVED)
	{
		Com_Printf(" KMOD_RESERVED");
	}

	Com_Printf(" Q:0x%02x(%s)\n", key, Key_KeynumToString(key,qtrue));
}

/*
 * @brief translates SDL keyboard identifier to its Q3 counterpart
 */
static const char *IN_TranslateSDLToQ3Key(SDL_Keysym *keysym,
                                          keyNum_t *key, qboolean down)
{
	static unsigned char buf[2] = { '\0', '\0' };

	if (keysym->sym >= SDLK_SPACE && keysym->sym < SDLK_DELETE)
	{
		// These happen to match the ASCII chars
		*key = (int)keysym->sym;
		// SDL2 depreciates unicode... no shifted chars
		buf[0] = keysym->sym;
		if ( keysym->mod & (KMOD_RSHIFT | KMOD_LSHIFT) )
		{
			if (keysym->sym >= 'a' && 'z' >= keysym->sym)
				buf[0] -= 0x20;
			else if (keysym->sym = 0x2d)
				buf[0] = 0x5f; // "-" to "_"
		}
	}
	else
	{
		buf[0] = 0;
		switch (keysym->sym)
		{
		case SDLK_PAGEUP:       *key = K_PGUP;          break;
		case SDLK_KP_9:          *key = K_KP_PGUP;       break;
		case SDLK_PAGEDOWN:     *key = K_PGDN;          break;
		case SDLK_KP_3:          *key = K_KP_PGDN;       break;
		case SDLK_KP_7:          *key = K_KP_HOME;       break;
		case SDLK_HOME:         *key = K_HOME;          break;
		case SDLK_KP_1:          *key = K_KP_END;        break;
		case SDLK_END:          *key = K_END;           break;
		case SDLK_KP_4:          *key = K_KP_LEFTARROW;  break;
		case SDLK_LEFT:         *key = K_LEFTARROW;     break;
		case SDLK_KP_6:          *key = K_KP_RIGHTARROW; break;
		case SDLK_RIGHT:        *key = K_RIGHTARROW;    break;
		case SDLK_KP_2:          *key = K_KP_DOWNARROW;  break;
		case SDLK_DOWN:         *key = K_DOWNARROW;     break;
		case SDLK_KP_8:          *key = K_KP_UPARROW;    break;
		case SDLK_UP:           *key = K_UPARROW;       break;
		case SDLK_ESCAPE:       *key = K_ESCAPE;        break;
		case SDLK_KP_ENTER:     *key = K_KP_ENTER;      break;
		case SDLK_RETURN:       *key = K_ENTER;         break;
		case SDLK_TAB:          *key = K_TAB;           break;
		case SDLK_F1:           *key = K_F1;            break;
		case SDLK_F2:           *key = K_F2;            break;
		case SDLK_F3:           *key = K_F3;            break;
		case SDLK_F4:           *key = K_F4;            break;
		case SDLK_F5:           *key = K_F5;            break;
		case SDLK_F6:           *key = K_F6;            break;
		case SDLK_F7:           *key = K_F7;            break;
		case SDLK_F8:           *key = K_F8;            break;
		case SDLK_F9:           *key = K_F9;            break;
		case SDLK_F10:          *key = K_F10;           break;
		case SDLK_F11:          *key = K_F11;           break;
		case SDLK_F12:          *key = K_F12;           break;
		case SDLK_F13:          *key = K_F13;           break;
		case SDLK_F14:          *key = K_F14;           break;
		case SDLK_F15:          *key = K_F15;           break;

		case SDLK_BACKSPACE:
			*key = K_BACKSPACE;
			buf[0] = 8;
			break;
		case SDLK_KP_PERIOD:    *key = K_KP_DEL;        break;
		case SDLK_DELETE:       *key = K_DEL;           break;
		case SDLK_PAUSE:        *key = K_PAUSE;         break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:       *key = K_SHIFT;         break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:        *key = K_CTRL;          break;

		case SDLK_RALT:
		case SDLK_LALT:         *key = K_ALT;           break;

		case SDLK_KP_5:          *key = K_KP_5;          break;
		case SDLK_INSERT:       *key = K_INS;           break;
		case SDLK_KP_0:          *key = K_KP_INS;        break;
		case SDLK_KP_MULTIPLY:  *key = K_KP_STAR;       break;
		case SDLK_KP_PLUS:      *key = K_KP_PLUS;       break;
		case SDLK_KP_MINUS:     *key = K_KP_MINUS;      break;
		case SDLK_KP_DIVIDE:    *key = K_KP_SLASH;      break;

		case SDLK_MODE:         *key = K_MODE;          break;
		case SDLK_SYSREQ:       *key = K_SYSREQ;        break;
		case SDLK_MENU:         *key = K_MENU;          break;
		case SDLK_UNDO:         *key = K_UNDO;          break;
		case SDLK_CAPSLOCK:     *key = K_CAPSLOCK;      break;

		default:
			*key = 0;
			break;
		}
	}

	if (in_keyboardDebug->integer)
	{
		IN_PrintKey(keysym, *key, down);
	}

	return (char *)buf;
}

static void IN_GobbleMotionEvents(void)
{
	SDL_Event dummy[1];

	// Gobble any mouse motion events
	SDL_PumpEvents();
	while (SDL_PeepEvents(dummy, 1, SDL_GETEVENT,
				SDL_MOUSEMOTION, SDL_MOUSEMOTION))
	{
	}
}

static void IN_ActivateMouse(void)
{
	if (!mouseAvailable || !SDL_WasInit(SDL_INIT_VIDEO))
	{
		return;
	}

	if (!mouseActive)
	{
		SDL_SetWindowGrab( SDLvidscreen, SDL_TRUE );
		SDL_SetRelativeMouseMode( SDL_TRUE );
		IN_GobbleMotionEvents();
	}

	// in_nograb makes no sense in fullscreen mode
	if (!Cvar_VariableIntegerValue("r_fullscreen"))
	{
		if (in_nograb->modified || !mouseActive)
		{
			in_nograb->modified = qfalse;
			if (in_nograb->integer)
				SDL_SetWindowGrab( SDLvidscreen, SDL_FALSE );
			else
				SDL_SetWindowGrab( SDLvidscreen, SDL_TRUE );
		}
	}

	mouseActive = qtrue;
}

static void IN_DeactivateMouse(void)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		return;
	}

	SDL_SetRelativeMouseMode( SDL_FALSE );
	// Always show the cursor when the mouse is disabled,
	// but not when fullscreen
	if (!Cvar_VariableIntegerValue("r_fullscreen"))
		SDL_ShowCursor(1);

	if (!mouseAvailable)
	{
		return;
	}

	if (mouseActive)
	{
		IN_GobbleMotionEvents();
		SDL_SetWindowGrab( SDLvidscreen, SDL_FALSE );
		mouseActive = qfalse;
	}
}

// We translate axes movement into keypresses
static int joy_keys[16] =
{
	K_LEFTARROW, K_RIGHTARROW,
	K_UPARROW,   K_DOWNARROW,
	K_JOY16,     K_JOY17,
	K_JOY18,     K_JOY19,
	K_JOY20,     K_JOY21,
	K_JOY22,     K_JOY23,

	K_JOY24,     K_JOY25,
	K_JOY26,     K_JOY27
};

// translate hat events into keypresses
// the 4 highest buttons are used for the first hat ...
static int hat_keys[16] =
{
	K_JOY29, K_JOY30,
	K_JOY31, K_JOY32,
	K_JOY25, K_JOY26,
	K_JOY27, K_JOY28,
	K_JOY21, K_JOY22,
	K_JOY23, K_JOY24,
	K_JOY17, K_JOY18,
	K_JOY19, K_JOY20
};


struct
{
	qboolean buttons[16];  // !!! FIXME: these might be too many.
	unsigned int oldaxes;
	int oldaaxes[16];
	unsigned int oldhats;
} stick_state;

static void IN_InitJoystick(void)
{
	int  i          = 0;
	int  total      = 0;
	char buf[16384] = "";

	if (stick != NULL)
	{
		SDL_JoystickClose(stick);
	}

	stick = NULL;
	memset(&stick_state, '\0', sizeof(stick_state));

	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	{
		Com_DPrintf("Calling SDL_Init(SDL_INIT_JOYSTICK)...\n");
		if (SDL_Init(SDL_INIT_JOYSTICK) == -1)
		{
			Com_DPrintf("SDL_Init(SDL_INIT_JOYSTICK) failed: %s\n", SDL_GetError());
			return;
		}
		Com_DPrintf("SDL_Init(SDL_INIT_JOYSTICK) passed.\n");
	}

	total = SDL_NumJoysticks();
	Com_DPrintf("%d possible joysticks\n", total);

	if (0 == total)
		return;

	// Print list and build cvar to allow ui to select joystick.
	for (i = 0; i < total; i++)
	{
		SDL_Joystick *tmp_joystick = SDL_JoystickOpen(i);
		if (tmp_joystick) {
			Q_strcat(buf, sizeof(buf), SDL_JoystickName(tmp_joystick));
			Q_strcat(buf, sizeof(buf), "\n");
			SDL_JoystickClose(tmp_joystick);
		}
	}

	Cvar_Get("in_availableJoysticks", buf, CVAR_ROM);

	in_joystickNo = Cvar_Get("in_joystickNo", "0", CVAR_ARCHIVE);
	if (in_joystickNo->integer < 0 || in_joystickNo->integer >= total)
	{
		Cvar_Set("in_joystickNo", "0");
	}

	in_joystickUseAnalog = Cvar_Get("in_joystickUseAnalog", "0", CVAR_ARCHIVE);

	stick = SDL_JoystickOpen(in_joystickNo->integer);

	if (stick == NULL)
	{
		Com_DPrintf("No joystick opened.\n");
		return;
	}

	Com_DPrintf("Joystick %d opened\n", in_joystickNo->integer);
	Com_DPrintf("Name:       %s\n", SDL_JoystickName(stick));

	Com_DPrintf("Axes:       %d\n", SDL_JoystickNumAxes(stick));
	Com_DPrintf("Hats:       %d\n", SDL_JoystickNumHats(stick));
	Com_DPrintf("Buttons:    %d\n", SDL_JoystickNumButtons(stick));
	Com_DPrintf("Balls:      %d\n", SDL_JoystickNumBalls(stick));
	Com_DPrintf("Use Analog: %s\n", in_joystickUseAnalog->integer ? "Yes" : "No");

	SDL_JoystickEventState(SDL_QUERY);
}

static void IN_ShutdownJoystick(void)
{
	if (stick)
	{
		SDL_JoystickClose(stick);
		stick = NULL;
	}

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

void IN_JoyMove(void)
{
	qboolean     joy_pressed[ARRAY_LEN(joy_keys)];
	unsigned int axes  = 0;
	unsigned int hats  = 0;
	int          total = 0;
	int          i     = 0;

	if (!stick)
	{
		return;
	}

	SDL_JoystickUpdate();

	memset(joy_pressed, '\0', sizeof(joy_pressed));

	// update the ball state.
	total = SDL_JoystickNumBalls(stick);
	if (total > 0)
	{
		int balldx = 0;
		int balldy = 0;
		for (i = 0; i < total; i++)
		{
			int dx = 0;
			int dy = 0;
			SDL_JoystickGetBall(stick, i, &dx, &dy);
			balldx += dx;
			balldy += dy;
		}
		if (balldx || balldy)
		{
			// !!! FIXME: is this good for stick balls, or just mice?
			// Scale like the mouse input...
			if (abs(balldx) > 1)
			{
				balldx *= 2;
			}
			if (abs(balldy) > 1)
			{
				balldy *= 2;
			}
			Com_QueueEvent(0, SE_MOUSE, balldx, balldy, 0, NULL);
		}
	}

	// now query the stick buttons...
	total = SDL_JoystickNumButtons(stick);
	if (total > 0)
	{
		if (total > ARRAY_LEN(stick_state.buttons))
		{
			total = ARRAY_LEN(stick_state.buttons);
		}
		for (i = 0; i < total; i++)
		{
			qboolean pressed = (SDL_JoystickGetButton(stick, i) != 0);
			if (pressed != stick_state.buttons[i])
			{
				Com_QueueEvent(0, SE_KEY, K_JOY1 + i, pressed, 0, NULL);
				stick_state.buttons[i] = pressed;
			}
		}
	}

	// look at the hats...
	total = SDL_JoystickNumHats(stick);
	if (total > 0)
	{
		if (total > 4)
		{
			total = 4;
		}
		for (i = 0; i < total; i++)
		{
			((Uint8 *)&hats)[i] = SDL_JoystickGetHat(stick, i);
		}
	}

	// update hat state
	if (hats != stick_state.oldhats)
	{
		for (i = 0; i < 4; i++)
		{
			if (((Uint8 *)&hats)[i] != ((Uint8 *)&stick_state.oldhats)[i])
			{
				// release event
				switch (((Uint8 *)&stick_state.oldhats)[i])
				{
				case SDL_HAT_UP:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 0], qfalse, 0, NULL);
					break;
				case SDL_HAT_RIGHT:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 1], qfalse, 0, NULL);
					break;
				case SDL_HAT_DOWN:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 2], qfalse, 0, NULL);
					break;
				case SDL_HAT_LEFT:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 3], qfalse, 0, NULL);
					break;
				case SDL_HAT_RIGHTUP:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 0], qfalse, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 1], qfalse, 0, NULL);
					break;
				case SDL_HAT_RIGHTDOWN:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 2], qfalse, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 1], qfalse, 0, NULL);
					break;
				case SDL_HAT_LEFTUP:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 0], qfalse, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 3], qfalse, 0, NULL);
					break;
				case SDL_HAT_LEFTDOWN:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 2], qfalse, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 3], qfalse, 0, NULL);
					break;
				default:
					break;
				}
				// press event
				switch (((Uint8 *)&hats)[i])
				{
				case SDL_HAT_UP:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 0], qtrue, 0, NULL);
					break;
				case SDL_HAT_RIGHT:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 1], qtrue, 0, NULL);
					break;
				case SDL_HAT_DOWN:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 2], qtrue, 0, NULL);
					break;
				case SDL_HAT_LEFT:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 3], qtrue, 0, NULL);
					break;
				case SDL_HAT_RIGHTUP:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 0], qtrue, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 1], qtrue, 0, NULL);
					break;
				case SDL_HAT_RIGHTDOWN:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 2], qtrue, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 1], qtrue, 0, NULL);
					break;
				case SDL_HAT_LEFTUP:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 0], qtrue, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 3], qtrue, 0, NULL);
					break;
				case SDL_HAT_LEFTDOWN:
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 2], qtrue, 0, NULL);
					Com_QueueEvent(0, SE_KEY, hat_keys[4 * i + 3], qtrue, 0, NULL);
					break;
				default:
					break;
				}
			}
		}
	}

	// save hat state
	stick_state.oldhats = hats;

	// finally, look at the axes...
	total = SDL_JoystickNumAxes(stick);
	if (total > 0)
	{
		if (total > 16)
		{
			total = 16;
		}
		for (i = 0; i < total; i++)
		{
			Sint16 axis = SDL_JoystickGetAxis(stick, i);

			if (in_joystickUseAnalog->integer)
			{
				float f = ((float) abs(axis)) / 32767.0f;

				if (f < in_joystickThreshold->value)
				{
					axis = 0;
				}

				if (axis != stick_state.oldaaxes[i])
				{
					Com_QueueEvent(0, SE_JOYSTICK_AXIS, i, axis, 0, NULL);
					stick_state.oldaaxes[i] = axis;
				}
			}
			else
			{
				float f = ((float) axis) / 32767.0f;
				if (f < -in_joystickThreshold->value)
				{
					axes |= (1 << (i * 2));
				}
				else if (f > in_joystickThreshold->value)
				{
					axes |= (1 << ((i * 2) + 1));
				}
			}
		}
	}

	/* Time to update axes state based on old vs. new. */
	if (axes != stick_state.oldaxes)
	{
		for (i = 0; i < 16; i++)
		{
			if ((axes & (1 << i)) && !(stick_state.oldaxes & (1 << i)))
			{
				Com_QueueEvent(0, SE_KEY, joy_keys[i], qtrue, 0, NULL);
			}

			if (!(axes & (1 << i)) && (stick_state.oldaxes & (1 << i)))
			{
				Com_QueueEvent(0, SE_KEY, joy_keys[i], qfalse, 0, NULL);
			}
		}
	}

	/* Save for future generations. */
	stick_state.oldaxes = axes;
}

static void IN_ProcessEvents(void)
{
	static keyNum_t scrollwheel = 0;
	SDL_Event  e;
	const char *character = NULL;
	keyNum_t   key        = 0;
	unsigned char b;

	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		return;
	}

	if ( scrollwheel )
	{
		// release mouse wheel "key"
		Com_QueueEvent(0, SE_KEY, scrollwheel, qfalse, 0, NULL);
		scrollwheel = 0;
	}

#if 0
	if (Key_GetCatcher() == 0 && keyRepeatEnabled)
	{
		SDL_EnableKeyRepeat(0, 0);
		keyRepeatEnabled = qfalse;
	}
	else if (!keyRepeatEnabled)
	{
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
		                    SDL_DEFAULT_REPEAT_INTERVAL);
		keyRepeatEnabled = qtrue;
	}
#endif

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_KEYDOWN:
			character = IN_TranslateSDLToQ3Key(&e.key.keysym, &key, qtrue);
			if (key)
			{
				Com_QueueEvent(0, SE_KEY, key, qtrue, 0, NULL);
			}

			if (character)
			{
				Com_QueueEvent(0, SE_CHAR, *character, 0, 0, NULL);
			}
			break;

		case SDL_KEYUP:
			character = IN_TranslateSDLToQ3Key(&e.key.keysym, &key, qfalse);
			if (key)
			{
				Com_QueueEvent(0, SE_KEY, key, qfalse, 0, NULL);
			}
			break;

		case SDL_MOUSEMOTION:
			if (mouseActive)
			{
				Com_QueueEvent(0, SE_MOUSE, e.motion.xrel, e.motion.yrel, 0, NULL);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case 1:   b = K_MOUSE1;     break;
			case 2:   b = K_MOUSE3;     break;
			case 3:   b = K_MOUSE2;     break;
			case 4:   b = K_MOUSE4;     break;
			case 5:   b = K_MOUSE5;     break;
			default:  b = K_AUX1 + (e.button.button - 8) % 16; break;
			}
			Com_QueueEvent(0, SE_KEY, b,
			               (e.type == SDL_MOUSEBUTTONDOWN ? qtrue : qfalse), 0, NULL);
			break;

        case SDL_MOUSEWHEEL:
			if (e.wheel.y > 0)
				scrollwheel = K_MWHEELUP;
			else
				scrollwheel = K_MWHEELDOWN;
			// fake mouse wheel "key"
			Com_QueueEvent(0, SE_KEY, scrollwheel, qtrue, 0, NULL);
			break;

		case SDL_QUIT:
			Cbuf_ExecuteText(EXEC_NOW, "quit Closed window\n");
			break;

		case SDL_WINDOWEVENT_RESIZED:
		{
			char width[32], height[32];
			Com_sprintf(width, sizeof(width), "%d", e.window.data1);
			Com_sprintf(height, sizeof(height), "%d", e.window.data2);
			Cvar_Set("r_customwidth", width);
			Cvar_Set("r_customheight", height);
			Cvar_Set("r_mode", "-1");
			/* wait until user stops dragging for 1 second, so
			   we aren't constantly recreating the GL context while
			   he tries to drag...*/
			vidRestartTime = Sys_Milliseconds() + 1000;
		}
		break;
		case SDL_WINDOWEVENT:
			if (e.window.event & SDL_WINDOW_INPUT_FOCUS)
			{
				Cvar_SetValue("com_unfocused", !(SDL_GetWindowFlags( SDLvidscreen ) & SDL_WINDOW_INPUT_FOCUS) );
			}
			if (e.window.event & (SDL_WINDOW_SHOWN | SDL_WINDOW_MINIMIZED))
			{
				Cvar_SetValue("com_minimized", !(SDL_GetWindowFlags( SDLvidscreen ) & SDL_WINDOW_MINIMIZED) );
				//  if ( e.active.gain && Cvar_VariableIntegerValue("r_fullscreen") )
				//      Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
			}
			break;

		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:
		default:
			break;
		}
	}
}

void IN_Frame(void)
{
	qboolean loading;
	qboolean fullscreen = qtrue;

	IN_JoyMove();
	IN_ProcessEvents();

	// If not DISCONNECTED (main menu) or ACTIVE (in game), we're loading
	loading = !!(cls.state != CA_DISCONNECTED && cls.state != CA_ACTIVE);

	if (!fullscreen && (  (Key_GetCatcher() & KEYCATCH_CONSOLE) ||
			loading || !SDL_GetWindowGrab(SDLvidscreen)  ))
	{
		if (mouseActive)
			IN_DeactivateMouse();
	}
	else
	{
		if (!mouseActive)
			IN_ActivateMouse();
	}

	/* in case we had to delay actual restart of video system... */
	if ((vidRestartTime != 0) && (vidRestartTime < Sys_Milliseconds()))
	{
		vidRestartTime = 0;
		Cbuf_AddText("vid_restart");
	}
}

void IN_Init(void)
{
	int flags;

	if ( !SDL_WasInit(SDL_INIT_VIDEO) )
	{
		Com_Error(ERR_FATAL, "IN_Init called before SDL_Init( SDL_INIT_VIDEO )\n");
		return;
	}

	Com_DPrintf("\n------- Input Initialization -------\n");

	in_keyboardDebug = Cvar_Get("in_keyboardDebug", "0", CVAR_ARCHIVE);

	// mouse variables
	in_mouse  = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);
	in_nograb = Cvar_Get("in_nograb", "0", CVAR_ARCHIVE);

	in_joystick          = Cvar_Get("in_joystick", "0", CVAR_ARCHIVE | CVAR_LATCH);
	in_joystickDebug     = Cvar_Get("in_joystickDebug", "0", CVAR_TEMP);
	in_joystickThreshold = Cvar_Get("joy_threshold", "0.15", CVAR_ARCHIVE);

/* Depreciated calls:
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	keyRepeatEnabled = qtrue;
*/

	if (in_mouse->value)
	{
		mouseAvailable = qtrue;
		IN_ActivateMouse();
	}
	else
	{
		IN_DeactivateMouse();
		mouseAvailable = qfalse;
	}

	flags = SDL_GetWindowFlags( SDLvidscreen );
	Cvar_SetValue("com_unfocused", !(flags & SDL_WINDOW_INPUT_FOCUS) );
	Cvar_SetValue("com_minimized", !(flags & SDL_WINDOW_MINIMIZED) );

	IN_InitJoystick();
}

void IN_Shutdown(void)
{
	IN_DeactivateMouse();
	mouseAvailable = qfalse;

	IN_ShutdownJoystick();
}

void IN_Restart(void)
{
	IN_ShutdownJoystick();
	IN_Init();
}
