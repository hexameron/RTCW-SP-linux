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

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "../game/q_shared.h"
#include "../game/g_public.h"
#include "../game/bg_public.h"
#include "../cgame/cg_public.h"
#include "../ui/ui_public.h"
#include "../cgame/tr_types.h"
#include "../qcommon/qcommon.h"

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;
// void CG_DrawInformation( void );
intptr_t CL_UISystemCalls( intptr_t *args );
intptr_t SV_GameSystemCalls( intptr_t *args );

intptr_t ui_call( intptr_t arg, ... ) {
	intptr_t args[8];
	int i;
	va_list ap;

	args[0] = arg;

	va_start( ap, arg );
	for ( i = 1; i < sizeof( args ) / sizeof( args[i] ); i++ )
		args[i] = va_arg( ap, intptr_t );
	va_end( ap );

	return CL_UISystemCalls( args );
}

intptr_t game_call( intptr_t arg, ... ) {
	intptr_t args[8];
	int i;
	va_list ap;

	args[0] = arg;

	va_start( ap, arg );
	for ( i = 1; i < sizeof( args ) / sizeof( args[i] ); i++ )
		args[i] = va_arg( ap, intptr_t );
	va_end( ap );

	return SV_GameSystemCalls( args );
}

int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void    trap_Printf( const char *fmt ) {
	Com_Printf("%s", fmt);
}

void    trap_Error( const char *fmt ) {
	Com_Error( ERR_DROP, "%s", fmt);
}

void    trap_Endgame( void ) {
	Com_Error( ERR_ENDGAME, "endgame" );
}

int     trap_Milliseconds( void ) {
	return Sys_Milliseconds();
}
int     trap_Argc( void ) {
	return Cmd_Argc();
}

void    trap_Argv( int n, char *buffer, int bufferLength ) {
	Cmd_ArgvBuffer( n, buffer, bufferLength );
}

int     trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return game_call( G_FS_FOPEN_FILE, qpath, f, mode );
}

void    trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	game_call( G_FS_READ, buffer, len, f );
}

int     trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return game_call( G_FS_WRITE, buffer, len, f );
}

int     trap_FS_Rename( const char *from, const char *to ) {
	return game_call( G_FS_RENAME, from, to );
}

void    trap_FS_FCloseFile( fileHandle_t f ) {
	game_call( G_FS_FCLOSE_FILE, f );
}

void    trap_FS_CopyFile( char *from, char *to ) {  //DAJ
	game_call( G_FS_COPY_FILE, from, to );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return game_call( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

void    trap_SendConsoleCommand( int exec_when, const char *text ) {
	game_call( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

void    trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	Cvar_Register( cvar, var_name, value, flags );
}

void    trap_Cvar_Update( vmCvar_t *cvar ) {
	game_call( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	game_call( G_CVAR_SET, var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return game_call( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}


void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						  playerState_t *clients, int sizeofGClient ) {
	game_call( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( int clientNum, const char *reason ) {
	game_call( G_DROP_CLIENT, clientNum, reason );
}

void trap_SendServerCommand( int clientNum, const char *text ) {
	game_call( G_SEND_SERVER_COMMAND, clientNum, text );
}

void trap_SetConfigstring( int num, const char *string ) {
	game_call( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	game_call( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	game_call( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
	game_call( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	game_call( G_GET_SERVERINFO, buffer, bufferSize );
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	game_call( G_SET_BRUSH_MODEL, ent, name );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	game_call( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	game_call( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return game_call( G_POINT_CONTENTS, point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return game_call( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return game_call( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	game_call( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return game_call( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntity( gentity_t *ent ) {
	game_call( G_LINKENTITY, ent );
}

void trap_UnlinkEntity( gentity_t *ent ) {
	game_call( G_UNLINKENTITY, ent );
}


int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return game_call( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return game_call( G_ENTITY_CONTACT, mins, maxs, ent );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return game_call( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

int trap_BotAllocateClient( void ) {
	return game_call( G_BOT_ALLOCATE_CLIENT );
}

void trap_BotFreeClient( int clientNum ) {
	game_call( G_BOT_FREE_CLIENT, clientNum );
}

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	game_call( G_GET_USERCMD, clientNum, cmd );
}

/* Duplicate call name, renamed for CGame code */
qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return game_call( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

int trap_DebugPolygonCreate( int color, int numPoints, vec3_t *points ) {
	return game_call( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete( int id ) {
	game_call( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime ) {
	return game_call( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	game_call( G_SNAPVECTOR, v );
	return;
}

qboolean trap_GetTag( int clientNum, char *tagName, orientation_t *or ) {
	return game_call( G_GETTAG, clientNum, tagName, or );
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return game_call( BOTLIB_SETUP );
}

int trap_BotLibShutdown( void ) {
	return game_call( BOTLIB_SHUTDOWN );
}

int trap_BotLibVarSet( char *var_name, char *value ) {
	return game_call( BOTLIB_LIBVAR_SET, var_name, value );
}

int trap_BotLibVarGet( char *var_name, char *value, int size ) {
	return game_call( BOTLIB_LIBVAR_GET, var_name, value, size );
}

int trap_BotLibDefine( char *string ) {
	return game_call( BOTLIB_PC_ADD_GLOBAL_DEFINE, string );
}

int trap_BotLibStartFrame( float time ) {
	return game_call( BOTLIB_START_FRAME, PASSFLOAT( time ) );
}

int trap_BotLibLoadMap( const char *mapname ) {
	return game_call( BOTLIB_LOAD_MAP, mapname );
}

int trap_BotLibUpdateEntity( int ent, void /* struct bot_updateentity_s */ *bue ) {
	return game_call( BOTLIB_UPDATENTITY, ent, bue );
}

int trap_BotLibTest( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 ) {
	return game_call( BOTLIB_TEST, parm0, parm1, parm2, parm3 );
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return game_call( BOTLIB_GET_SNAPSHOT_ENTITY, clientNum, sequence );
}

int trap_BotGetServerCommand( int clientNum, char *message, int size ) {
	return game_call( BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size );
}

void trap_BotUserCommand( int clientNum, usercmd_t *ucmd ) {
	game_call( BOTLIB_USER_COMMAND, clientNum, ucmd );
}

void trap_AAS_EntityInfo( int entnum, void /* struct aas_entityinfo_s */ *info ) {
	game_call( BOTLIB_AAS_ENTITY_INFO, entnum, info );
}

int trap_AAS_Initialized( void ) {
	return game_call( BOTLIB_AAS_INITIALIZED );
}

void trap_AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs ) {
	game_call( BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX, presencetype, mins, maxs );
}

float trap_AAS_Time( void ) {
	int temp;
	temp = game_call( BOTLIB_AAS_TIME );
	return ( *(float*)&temp );
}

// Ridah, multiple AAS files
void trap_AAS_SetCurrentWorld( int index ) {
	game_call( BOTLIB_AAS_SETCURRENTWORLD, index );
}
// done.

int trap_AAS_PointAreaNum( vec3_t point ) {
	return game_call( BOTLIB_AAS_POINT_AREA_NUM, point );
}

int trap_AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas ) {
	return game_call( BOTLIB_AAS_TRACE_AREAS, start, end, areas, points, maxareas );
}

int trap_AAS_PointContents( vec3_t point ) {
	return game_call( BOTLIB_AAS_POINT_CONTENTS, point );
}

int trap_AAS_NextBSPEntity( int ent ) {
	return game_call( BOTLIB_AAS_NEXT_BSP_ENTITY, ent );
}

int trap_AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size ) {
	return game_call( BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY, ent, key, value, size );
}

int trap_AAS_VectorForBSPEpairKey( int ent, char *key, vec3_t v ) {
	return game_call( BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY, ent, key, v );
}

int trap_AAS_FloatForBSPEpairKey( int ent, char *key, float *value ) {
	return game_call( BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY, ent, key, value );
}

int trap_AAS_IntForBSPEpairKey( int ent, char *key, int *value ) {
	return game_call( BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY, ent, key, value );
}

int trap_AAS_AreaReachability( int areanum ) {
	return game_call( BOTLIB_AAS_AREA_REACHABILITY, areanum );
}

int trap_AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags ) {
	return game_call( BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA, areanum, origin, goalareanum, travelflags );
}

int trap_AAS_Swimming( vec3_t origin ) {
	return game_call( BOTLIB_AAS_SWIMMING, origin );
}

int AAS_PredictClientMovement( void /*aas_clientmove_s*/ *move,
							   int entnum, vec3_t origin,
							   int presencetype, int onground,
							   vec3_t velocity, vec3_t cmdmove,
							   int cmdframes,
							   int maxframes, float frametime,
							   int stopevent, int stopareanum, int visualize );
	
int trap_AAS_PredictClientMovement( void *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove,
											int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize ) {
	return AAS_PredictClientMovement( move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, visualize );
}

// Ridah, route-tables
void trap_AAS_RT_ShowRoute( vec3_t srcpos, int srcnum, int destnum ) {
	game_call( BOTLIB_AAS_RT_SHOWROUTE, srcpos, srcnum, destnum );
}

qboolean trap_AAS_RT_GetHidePos( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos ) {
	return game_call( BOTLIB_AAS_RT_GETHIDEPOS, srcpos, srcnum, srcarea, destpos, destnum, destarea, returnPos );
}

int trap_AAS_FindAttackSpotWithinRange( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos ) {
	return game_call( BOTLIB_AAS_FINDATTACKSPOTWITHINRANGE, srcnum, rangenum, enemynum, PASSFLOAT( rangedist ), travelflags, outpos );
}

qboolean trap_AAS_GetRouteFirstVisPos( vec3_t srcpos, vec3_t destpos, int travelflags, vec3_t retpos ) {
	return game_call( BOTLIB_AAS_GETROUTEFIRSTVISPOS, srcpos, destpos, travelflags, retpos );
}

void trap_AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, qboolean blocking ) {
	game_call( BOTLIB_AAS_SETAASBLOCKINGENTITY, absmin, absmax, blocking );
}
// done.

void trap_EA_Say( int client, char *str ) {
	game_call( BOTLIB_EA_SAY, client, str );
}

void trap_EA_SayTeam( int client, char *str ) {
	game_call( BOTLIB_EA_SAY_TEAM, client, str );
}

void trap_EA_UseItem( int client, char *it ) {
	game_call( BOTLIB_EA_USE_ITEM, client, it );
}

void trap_EA_DropItem( int client, char *it ) {
	game_call( BOTLIB_EA_DROP_ITEM, client, it );
}

void trap_EA_UseInv( int client, char *inv ) {
	game_call( BOTLIB_EA_USE_INV, client, inv );
}

void trap_EA_DropInv( int client, char *inv ) {
	game_call( BOTLIB_EA_DROP_INV, client, inv );
}

void trap_EA_Gesture( int client ) {
	game_call( BOTLIB_EA_GESTURE, client );
}

void trap_EA_Command( int client, char *command ) {
	game_call( BOTLIB_EA_COMMAND, client, command );
}

void trap_EA_SelectWeapon( int client, int weapon ) {
	game_call( BOTLIB_EA_SELECT_WEAPON, client, weapon );
}

void trap_EA_Talk( int client ) {
	game_call( BOTLIB_EA_TALK, client );
}

void trap_EA_Attack( int client ) {
	game_call( BOTLIB_EA_ATTACK, client );
}

void trap_EA_Reload( int client ) {
	game_call( BOTLIB_EA_RELOAD, client );
}

void trap_EA_Use( int client ) {
	game_call( BOTLIB_EA_USE, client );
}

void trap_EA_Respawn( int client ) {
	game_call( BOTLIB_EA_RESPAWN, client );
}

void trap_EA_Jump( int client ) {
	game_call( BOTLIB_EA_JUMP, client );
}

void trap_EA_DelayedJump( int client ) {
	game_call( BOTLIB_EA_DELAYED_JUMP, client );
}

void trap_EA_Crouch( int client ) {
	game_call( BOTLIB_EA_CROUCH, client );
}

void trap_EA_MoveUp( int client ) {
	game_call( BOTLIB_EA_MOVE_UP, client );
}

void trap_EA_MoveDown( int client ) {
	game_call( BOTLIB_EA_MOVE_DOWN, client );
}

void trap_EA_MoveForward( int client ) {
	game_call( BOTLIB_EA_MOVE_FORWARD, client );
}

void trap_EA_MoveBack( int client ) {
	game_call( BOTLIB_EA_MOVE_BACK, client );
}

void trap_EA_MoveLeft( int client ) {
	game_call( BOTLIB_EA_MOVE_LEFT, client );
}

void trap_EA_MoveRight( int client ) {
	game_call( BOTLIB_EA_MOVE_RIGHT, client );
}

void trap_EA_Move( int client, vec3_t dir, float speed ) {
	game_call( BOTLIB_EA_MOVE, client, dir, PASSFLOAT( speed ) );
}

void trap_EA_View( int client, vec3_t viewangles ) {
	game_call( BOTLIB_EA_VIEW, client, viewangles );
}

void trap_EA_EndRegular( int client, float thinktime ) {
	game_call( BOTLIB_EA_END_REGULAR, client, PASSFLOAT( thinktime ) );
}

void trap_EA_GetInput( int client, float thinktime, void /* struct bot_input_s */ *input ) {
	game_call( BOTLIB_EA_GET_INPUT, client, PASSFLOAT( thinktime ), input );
}

void trap_EA_ResetInput( int client, void *init ) {
	game_call( BOTLIB_EA_RESET_INPUT, client, init );
}

int trap_BotLoadCharacter( char *charfile, int skill ) {
	return game_call( BOTLIB_AI_LOAD_CHARACTER, charfile, skill );
}

void trap_BotFreeCharacter( int character ) {
	game_call( BOTLIB_AI_FREE_CHARACTER, character );
}

float trap_Characteristic_Float( int character, int index ) {
	int temp;
	temp = game_call( BOTLIB_AI_CHARACTERISTIC_FLOAT, character, index );
	return ( *(float*)&temp );
}

float trap_Characteristic_BFloat( int character, int index, float min, float max ) {
	int temp;
	temp = game_call( BOTLIB_AI_CHARACTERISTIC_BFLOAT, character, index, PASSFLOAT( min ), PASSFLOAT( max ) );
	return ( *(float*)&temp );
}

int trap_Characteristic_Integer( int character, int index ) {
	return game_call( BOTLIB_AI_CHARACTERISTIC_INTEGER, character, index );
}

int trap_Characteristic_BInteger( int character, int index, int min, int max ) {
	return game_call( BOTLIB_AI_CHARACTERISTIC_BINTEGER, character, index, min, max );
}

void trap_Characteristic_String( int character, int index, char *buf, int size ) {
	game_call( BOTLIB_AI_CHARACTERISTIC_STRING, character, index, buf, size );
}

int trap_BotAllocChatState( void ) {
	return game_call( BOTLIB_AI_ALLOC_CHAT_STATE );
}

void trap_BotFreeChatState( int handle ) {
	game_call( BOTLIB_AI_FREE_CHAT_STATE, handle );
}

void trap_BotQueueConsoleMessage( int chatstate, int type, char *message ) {
	game_call( BOTLIB_AI_QUEUE_CONSOLE_MESSAGE, chatstate, type, message );
}

void trap_BotRemoveConsoleMessage( int chatstate, int handle ) {
	game_call( BOTLIB_AI_REMOVE_CONSOLE_MESSAGE, chatstate, handle );
}

int trap_BotNextConsoleMessage( int chatstate, void /* struct bot_consolemessage_s */ *cm ) {
	return game_call( BOTLIB_AI_NEXT_CONSOLE_MESSAGE, chatstate, cm );
}

int trap_BotNumConsoleMessages( int chatstate ) {
	return game_call( BOTLIB_AI_NUM_CONSOLE_MESSAGE, chatstate );
}

void BotInitialChat( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
void trap_BotInitialChat( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	BotInitialChat( chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int trap_BotNumInitialChats( int chatstate, char *type ) {
	return game_call( BOTLIB_AI_NUM_INITIAL_CHATS, chatstate, type );
}

int BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int trap_BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	return BotReplyChat( chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int trap_BotChatLength( int chatstate ) {
	return game_call( BOTLIB_AI_CHAT_LENGTH, chatstate );
}

void trap_BotEnterChat( int chatstate, int client, int sendto ) {
	game_call( BOTLIB_AI_ENTER_CHAT, chatstate, client, sendto );
}

void trap_BotGetChatMessage( int chatstate, char *buf, int size ) {
	game_call( BOTLIB_AI_GET_CHAT_MESSAGE, chatstate, buf, size );
}

int trap_StringContains( char *str1, char *str2, int casesensitive ) {
	return game_call( BOTLIB_AI_STRING_CONTAINS, str1, str2, casesensitive );
}

int trap_BotFindMatch( char *str, void /* struct bot_match_s */ *match, unsigned int context ) {
	return game_call( BOTLIB_AI_FIND_MATCH, str, match, context );
}

void trap_BotMatchVariable( void /* struct bot_match_s */ *match, int variable, char *buf, int size ) {
	game_call( BOTLIB_AI_MATCH_VARIABLE, match, variable, buf, size );
}

void trap_UnifyWhiteSpaces( char *string ) {
	game_call( BOTLIB_AI_UNIFY_WHITE_SPACES, string );
}

void trap_BotReplaceSynonyms( char *string, unsigned int context ) {
	game_call( BOTLIB_AI_REPLACE_SYNONYMS, string, context );
}

int trap_BotLoadChatFile( int chatstate, char *chatfile, char *chatname ) {
	return game_call( BOTLIB_AI_LOAD_CHAT_FILE, chatstate, chatfile, chatname );
}

void trap_BotSetChatGender( int chatstate, int gender ) {
	game_call( BOTLIB_AI_SET_CHAT_GENDER, chatstate, gender );
}

void trap_BotSetChatName( int chatstate, char *name ) {
	game_call( BOTLIB_AI_SET_CHAT_NAME, chatstate, name );
}

void trap_BotResetGoalState( int goalstate ) {
	game_call( BOTLIB_AI_RESET_GOAL_STATE, goalstate );
}

void trap_BotResetAvoidGoals( int goalstate ) {
	game_call( BOTLIB_AI_RESET_AVOID_GOALS, goalstate );
}

void trap_BotRemoveFromAvoidGoals( int goalstate, int number ) {
	game_call( BOTLIB_AI_REMOVE_FROM_AVOID_GOALS, goalstate, number );
}

void trap_BotPushGoal( int goalstate, void /* struct bot_goal_s */ *goal ) {
	game_call( BOTLIB_AI_PUSH_GOAL, goalstate, goal );
}

void trap_BotPopGoal( int goalstate ) {
	game_call( BOTLIB_AI_POP_GOAL, goalstate );
}

void trap_BotEmptyGoalStack( int goalstate ) {
	game_call( BOTLIB_AI_EMPTY_GOAL_STACK, goalstate );
}

void trap_BotDumpAvoidGoals( int goalstate ) {
	game_call( BOTLIB_AI_DUMP_AVOID_GOALS, goalstate );
}

void trap_BotDumpGoalStack( int goalstate ) {
	game_call( BOTLIB_AI_DUMP_GOAL_STACK, goalstate );
}

void trap_BotGoalName( int number, char *name, int size ) {
	game_call( BOTLIB_AI_GOAL_NAME, number, name, size );
}

int trap_BotGetTopGoal( int goalstate, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_GET_TOP_GOAL, goalstate, goal );
}

int trap_BotGetSecondGoal( int goalstate, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_GET_SECOND_GOAL, goalstate, goal );
}

int trap_BotChooseLTGItem( int goalstate, vec3_t origin, int *inventory, int travelflags ) {
	return game_call( BOTLIB_AI_CHOOSE_LTG_ITEM, goalstate, origin, inventory, travelflags );
}

int trap_BotChooseNBGItem( int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime ) {
	return game_call( BOTLIB_AI_CHOOSE_NBG_ITEM, goalstate, origin, inventory, travelflags, ltg, PASSFLOAT( maxtime ) );
}

int trap_BotTouchingGoal( vec3_t origin, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_TOUCHING_GOAL, origin, goal );
}

int trap_BotItemGoalInVisButNotVisible( int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE, viewer, eye, viewangles, goal );
}

int trap_BotGetLevelItemGoal( int index, char *classname, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_GET_LEVEL_ITEM_GOAL, index, classname, goal );
}

int trap_BotGetNextCampSpotGoal( int num, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL, num, goal );
}

int trap_BotGetMapLocationGoal( char *name, void /* struct bot_goal_s */ *goal ) {
	return game_call( BOTLIB_AI_GET_MAP_LOCATION_GOAL, name, goal );
}

float trap_BotAvoidGoalTime( int goalstate, int number ) {
	int temp;
	temp = game_call( BOTLIB_AI_AVOID_GOAL_TIME, goalstate, number );
	return ( *(float*)&temp );
}

void trap_BotInitLevelItems( void ) {
	game_call( BOTLIB_AI_INIT_LEVEL_ITEMS );
}

void trap_BotUpdateEntityItems( void ) {
	game_call( BOTLIB_AI_UPDATE_ENTITY_ITEMS );
}

int trap_BotLoadItemWeights( int goalstate, char *filename ) {
	return game_call( BOTLIB_AI_LOAD_ITEM_WEIGHTS, goalstate, filename );
}

void trap_BotFreeItemWeights( int goalstate ) {
	game_call( BOTLIB_AI_FREE_ITEM_WEIGHTS, goalstate );
}

void trap_BotInterbreedGoalFuzzyLogic( int parent1, int parent2, int child ) {
	game_call( BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC, parent1, parent2, child );
}

void trap_BotSaveGoalFuzzyLogic( int goalstate, char *filename ) {
	game_call( BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC, goalstate, filename );
}

void trap_BotMutateGoalFuzzyLogic( int goalstate, float range ) {
	game_call( BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC, goalstate, range );
}

int trap_BotAllocGoalState( int state ) {
	return game_call( BOTLIB_AI_ALLOC_GOAL_STATE, state );
}

void trap_BotFreeGoalState( int handle ) {
	game_call( BOTLIB_AI_FREE_GOAL_STATE, handle );
}

void trap_BotResetMoveState( int movestate ) {
	game_call( BOTLIB_AI_RESET_MOVE_STATE, movestate );
}

void trap_BotMoveToGoal( void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags ) {
	game_call( BOTLIB_AI_MOVE_TO_GOAL, result, movestate, goal, travelflags );
}

int trap_BotMoveInDirection( int movestate, vec3_t dir, float speed, int type ) {
	return game_call( BOTLIB_AI_MOVE_IN_DIRECTION, movestate, dir, PASSFLOAT( speed ), type );
}

void trap_BotResetAvoidReach( int movestate ) {
	game_call( BOTLIB_AI_RESET_AVOID_REACH, movestate );
}

void trap_BotResetLastAvoidReach( int movestate ) {
	game_call( BOTLIB_AI_RESET_LAST_AVOID_REACH,movestate  );
}

int trap_BotReachabilityArea( vec3_t origin, int testground ) {
	return game_call( BOTLIB_AI_REACHABILITY_AREA, origin, testground );
}

int trap_BotMovementViewTarget( int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target ) {
	return game_call( BOTLIB_AI_MOVEMENT_VIEW_TARGET, movestate, goal, travelflags, PASSFLOAT( lookahead ), target );
}

int trap_BotPredictVisiblePosition( vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target ) {
	return game_call( BOTLIB_AI_PREDICT_VISIBLE_POSITION, origin, areanum, goal, travelflags, target );
}

int trap_BotAllocMoveState( void ) {
	return game_call( BOTLIB_AI_ALLOC_MOVE_STATE );
}

void trap_BotFreeMoveState( int handle ) {
	game_call( BOTLIB_AI_FREE_MOVE_STATE, handle );
}

void trap_BotInitMoveState( int handle, void /* struct bot_initmove_s */ *initmove ) {
	game_call( BOTLIB_AI_INIT_MOVE_STATE, handle, initmove );
}

// Ridah
void trap_BotInitAvoidReach( int handle ) {
	game_call( BOTLIB_AI_INIT_AVOID_REACH, handle );
}
// Done.

int trap_BotChooseBestFightWeapon( int weaponstate, int *inventory ) {
	return game_call( BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON, weaponstate, inventory );
}

void trap_BotGetWeaponInfo( int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo ) {
	game_call( BOTLIB_AI_GET_WEAPON_INFO, weaponstate, weapon, weaponinfo );
}

int trap_BotLoadWeaponWeights( int weaponstate, char *filename ) {
	return game_call( BOTLIB_AI_LOAD_WEAPON_WEIGHTS, weaponstate, filename );
}

int trap_BotAllocWeaponState( void ) {
	return game_call( BOTLIB_AI_ALLOC_WEAPON_STATE );
}

void trap_BotFreeWeaponState( int weaponstate ) {
	game_call( BOTLIB_AI_FREE_WEAPON_STATE, weaponstate );
}

void trap_BotResetWeaponState( int weaponstate ) {
	game_call( BOTLIB_AI_RESET_WEAPON_STATE, weaponstate );
}

int trap_GeneticParentsAndChildSelection( int numranks, float *ranks, int *parent1, int *parent2, int *child ) {
	return game_call( BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION, numranks, ranks, parent1, parent2, child );
}

/*
===========================================================================
===========================================================================
*/

/* 
 * DUPLICATES
 *
void trap_Print( const char *string ) {
	ui_call( UI_PRINT, string );
}

void trap_Error( const char *string ) {
	ui_call( UI_ERROR, string );
}

int trap_Milliseconds( void ) {
	return ui_call( UI_MILLISECONDS );
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	ui_call( UI_CVAR_REGISTER, cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	ui_call( UI_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	ui_call( UI_CVAR_SET, var_name, value );
}
*/

float trap_Cvar_VariableValue( const char *var_name ) {
	int temp;
	temp = ui_call( UI_CVAR_VARIABLEVALUE, var_name );
	return ( *(float*)&temp );
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	ui_call( UI_CVAR_SETVALUE, var_name, PASSFLOAT( value ) );
}


/* 
 * DUPLICATES
 *
void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	ui_call( UI_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_Reset( const char *name ) {
	ui_call( UI_CVAR_RESET, name );
}

void trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	ui_call( UI_CVAR_CREATE, var_name, var_value, flags );
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	ui_call( UI_CVAR_INFOSTRINGBUFFER, bit, buffer, bufsize );
}

int trap_Argc( void ) {
	return ui_call( UI_ARGC );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	ui_call( UI_ARGV, n, buffer, bufferLength );
}
*/
void trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	ui_call( UI_CMD_EXECUTETEXT, exec_when, text );
}

/* 
 * DUPLICATES
 *
int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return ui_call( UI_FS_FOPENFILE, qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	ui_call( UI_FS_READ, buffer, len, f );
}

//----(SA)	added
void trap_FS_Seek( fileHandle_t f, int offset, int origin  ) {
	ui_call( UI_FS_SEEK, f, offset, origin );
}
//----(SA)	end

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	ui_call( UI_FS_WRITE, buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	ui_call( UI_FS_FCLOSEFILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return ui_call( UI_FS_GETFILELIST, path, extension, listbuf, bufsize );
}
*/
int trap_FS_Delete( const char *filename ) {
	return ui_call( UI_FS_DELETEFILE, filename );
}
/*
qhandle_t trap_R_RegisterModel( const char *name ) {
	return ui_call( UI_R_REGISTERMODEL, name );
}

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return ui_call( UI_R_REGISTERSKIN, name );
}

void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	ui_call( UI_R_REGISTERFONT, fontName, pointSize, font );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return ui_call( UI_R_REGISTERSHADERNOMIP, name );
}

void trap_R_ClearScene( void ) {
	ui_call( UI_R_CLEARSCENE );
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	ui_call( UI_R_ADDREFENTITYTOSCENE, re );
}

void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	ui_call( UI_R_ADDPOLYTOSCENE, hShader, numVerts, verts );
}

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
	ui_call( UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT( intensity ), PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), overdraw );
}

void trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, int flags ) {
	ui_call( UI_R_ADDCORONATOSCENE, org, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( scale ), id, flags );
}

void trap_R_RenderScene( const refdef_t *fd ) {
	ui_call( UI_R_RENDERSCENE, fd );
}

void trap_R_SetColor( const float *rgba ) {
	ui_call( UI_R_SETCOLOR, rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	ui_call( UI_R_DRAWSTRETCHPIC, PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader );
}

void    trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	ui_call( UI_R_MODELBOUNDS, model, mins, maxs );
}

void trap_UpdateScreen( void ) {
	ui_call( UI_UPDATESCREEN );
}
*/

int trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
	return ui_call( UI_CM_LERPTAG, tag, refent, tagName, 0 );           // NEFVE - SMF - fixed
}

/* 
 * DUPLICATES
 *
void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	ui_call( UI_S_STARTLOCALSOUND, sfx, channelNum );
}

sfxHandle_t trap_S_RegisterSound( const char *sample ) {
	return ui_call( UI_S_REGISTERSOUND, sample );
}

//----(SA)	added (already in cg)
void    trap_S_FadeBackgroundTrack( float targetvol, int time, int num ) {   // yes, i know.  fadebackground coming in, fadestreaming going out.  will have to see where functionality leads...
	ui_call( UI_S_FADESTREAMINGSOUND, PASSFLOAT( targetvol ), time, num ); // 'num' is '0' if it's music, '1' if it's "all streaming sounds"
}

void    trap_S_FadeAllSound( float targetvol, int time ) {
	ui_call( UI_S_FADEALLSOUNDS, PASSFLOAT( targetvol ), time );
}
//----(SA)	end
*/

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	ui_call( UI_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	ui_call( UI_KEY_GETBINDINGBUF, keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	ui_call( UI_KEY_SETBINDING, keynum, binding );
}

/* 
 * DUPLICATES
 *
qboolean trap_Key_IsDown( int keynum ) {
	return ui_call( UI_KEY_ISDOWN, keynum );
}
*/
qboolean trap_Key_GetOverstrikeMode( void ) {
	return ui_call( UI_KEY_GETOVERSTRIKEMODE );
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
	ui_call( UI_KEY_SETOVERSTRIKEMODE, state );
}

void trap_Key_ClearStates( void ) {
	ui_call( UI_KEY_CLEARSTATES );
}

int trap_Key_GetCatcher( void ) {
	return ui_call( UI_KEY_GETCATCHER );
}

void trap_Key_SetCatcher( int catcher ) {
	ui_call( UI_KEY_SETCATCHER, catcher );
}

void trap_GetClipboardData( char *buf, int bufsize ) {
	ui_call( UI_GETCLIPBOARDDATA, buf, bufsize );
}

void trap_GetClientState( uiClientState_t *state ) {
	ui_call( UI_GETCLIENTSTATE, state );
}

/* 
 * DUPLICATES
 *
v
void trap_GetGlconfig( glconfig_t *glconfig ) {
	ui_call( UI_GETGLCONFIG, glconfig );
}
*/
int trap_GetConfigString( int index, char* buff, int buffsize ) {
	return ui_call( UI_GETCONFIGSTRING, index, buff, buffsize );
}

int trap_LAN_GetLocalServerCount( void ) {
	return ui_call( UI_LAN_GETLOCALSERVERCOUNT );
}

void trap_LAN_GetLocalServerAddressString( int n, char *buf, int buflen ) {
	ui_call( UI_LAN_GETLOCALSERVERADDRESSSTRING, n, buf, buflen );
}

int trap_LAN_GetGlobalServerCount( void ) {
	return ui_call( UI_LAN_GETGLOBALSERVERCOUNT );
}

void trap_LAN_GetGlobalServerAddressString( int n, char *buf, int buflen ) {
	ui_call( UI_LAN_GETGLOBALSERVERADDRESSSTRING, n, buf, buflen );
}

int trap_LAN_GetPingQueueCount( void ) {
	return ui_call( UI_LAN_GETPINGQUEUECOUNT );
}

void trap_LAN_ClearPing( int n ) {
	ui_call( UI_LAN_CLEARPING, n );
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	ui_call( UI_LAN_GETPING, n, buf, buflen, pingtime );
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	ui_call( UI_LAN_GETPINGINFO, n, buf, buflen );
}

// NERVE - SMF
qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return ui_call( UI_LAN_UPDATEVISIBLEPINGS, source );
}

int trap_LAN_GetServerCount( int source ) {
	return ui_call( UI_LAN_GETSERVERCOUNT, source );
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return ui_call( UI_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2 );
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	ui_call( UI_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	ui_call( UI_LAN_GETSERVERINFO, source, n, buf, buflen );
}

int trap_LAN_AddServer( int source, const char *name, const char *addr ) {
	return ui_call( UI_LAN_ADDSERVER, source, name, addr );
}

void trap_LAN_RemoveServer( int source, const char *addr ) {
	ui_call( UI_LAN_REMOVESERVER, source, addr );
}

int trap_LAN_GetServerPing( int source, int n ) {
	return ui_call( UI_LAN_GETSERVERPING, source, n );
}

int trap_LAN_ServerIsVisible( int source, int n ) {
	return ui_call( UI_LAN_SERVERISVISIBLE, source, n );
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return ui_call( UI_LAN_SERVERSTATUS, serverAddress, serverStatus, maxLen );
}

void trap_LAN_SaveCachedServers() {
	ui_call( UI_LAN_SAVECACHEDSERVERS );
}

void trap_LAN_LoadCachedServers() {
	ui_call( UI_LAN_LOADCACHEDSERVERS );
}

void trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	ui_call( UI_LAN_MARKSERVERVISIBLE, source, n, visible );
}

void trap_LAN_ResetPings( int n ) {
	ui_call( UI_LAN_RESETPINGS, n );
}
// -NERVE - SMF

/* 
 * DUPLICATES
 *
int trap_MemoryRemaining( void ) {
	return ui_call( UI_MEMORY_REMAINING );
}
*/

void trap_GetCDKey( char *buf, int buflen ) {
	ui_call( UI_GET_CDKEY, buf, buflen );
}

void trap_SetCDKey( char *buf ) {
	ui_call( UI_SET_CDKEY, buf );
}

/* 
 * DUPLICATES
 *
int trap_PC_AddGlobalDefine( char *define ) {
	return ui_call( UI_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return ui_call( UI_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return ui_call( UI_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return ui_call( UI_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return ui_call( UI_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

void trap_S_StopBackgroundTrack( void ) {
	ui_call( UI_S_STOPBACKGROUNDTRACK );
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	ui_call( UI_S_STARTBACKGROUNDTRACK, intro, loop, fadeupTime );
}

int trap_RealTime( qtime_t *qtime ) {
	return ui_call( UI_REAL_TIME, qtime );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return ui_call( UI_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits );
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic( int handle ) {
	return ui_call( UI_CIN_STOPCINEMATIC, handle );
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic( int handle ) {
	return ui_call( UI_CIN_RUNCINEMATIC, handle );
}


// draws the current frame
void trap_CIN_DrawCinematic( int handle ) {
	ui_call( UI_CIN_DRAWCINEMATIC, handle );
}


// allows you to resize the animation dynamically
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	ui_call( UI_CIN_SETEXTENTS, handle, x, y, w, h );
}


void    trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	ui_call( UI_R_REMAP_SHADER, oldShader, newShader, timeOffset );
}
*/

qboolean trap_VerifyCDKey( const char *key, const char *chksum ) {
	return ui_call( UI_VERIFY_CDKEY, key, chksum );
}

// NERVE - SMF
qboolean trap_GetLimboString( int index, char *buf ) {
	return ui_call( UI_CL_GETLIMBOSTRING, index, buf );
}

