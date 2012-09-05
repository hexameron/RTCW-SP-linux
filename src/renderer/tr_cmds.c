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

/* This file is depreciated : it exists only to queue commands for a second CPU. There should
	be advantages to passing commands directly to the GPU. */ 

#include "tr_local.h"
volatile qboolean renderThreadActive;

/*
=====================
R_PerformanceCounters
=====================
*/
void R_PerformanceCounters( void ) {
	if ( !r_speeds->integer ) {
		// clear the counters even if we aren't printing
		memset( &tr.pc, 0, sizeof( tr.pc ) );
		memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
		return;
	}

	if ( r_speeds->integer == 1 ) {
		ri.Printf( PRINT_ALL, "%i/%i shaders/surfs %i leafs %i verts %i/%i tris %.2f mtex %.2f dc\n",
				   backEnd.pc.c_shaders, backEnd.pc.c_surfaces, tr.pc.c_leafs, backEnd.pc.c_vertexes,
				   backEnd.pc.c_indexes / 3, backEnd.pc.c_totalIndexes / 3,
				   R_SumOfUsedImages() / ( 1000000.0f ), backEnd.pc.c_overDraw / (float)( glConfig.vidWidth * glConfig.vidHeight ) );
	} else if ( r_speeds->integer == 2 ) {
		ri.Printf( PRINT_ALL, "(patch) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
				   tr.pc.c_sphere_cull_patch_in, tr.pc.c_sphere_cull_patch_clip, tr.pc.c_sphere_cull_patch_out,
				   tr.pc.c_box_cull_patch_in, tr.pc.c_box_cull_patch_clip, tr.pc.c_box_cull_patch_out );
		ri.Printf( PRINT_ALL, "(md3) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
				   tr.pc.c_sphere_cull_md3_in, tr.pc.c_sphere_cull_md3_clip, tr.pc.c_sphere_cull_md3_out,
				   tr.pc.c_box_cull_md3_in, tr.pc.c_box_cull_md3_clip, tr.pc.c_box_cull_md3_out );
	} else if ( r_speeds->integer == 3 ) {
		ri.Printf( PRINT_ALL, "viewcluster: %i\n", tr.viewCluster );
	} else if ( r_speeds->integer == 4 ) {
		if ( backEnd.pc.c_dlightVertexes ) {
			ri.Printf( PRINT_ALL, "dlight srf:%i  culled:%i  verts:%i  tris:%i\n",
					   tr.pc.c_dlightSurfaces, tr.pc.c_dlightSurfacesCulled,
					   backEnd.pc.c_dlightVertexes, backEnd.pc.c_dlightIndexes / 3 );
		}
	}
//----(SA)	this is unnecessary since it will always show 2048.  I moved this to where it is accurate for the world
//	else if (r_speeds->integer == 5 )
//	{
//		ri.Printf( PRINT_ALL, "zFar: %.0f\n", tr.viewParms.zFar );
//	}
	else if ( r_speeds->integer == 6 ) {
		ri.Printf( PRINT_ALL, "flare adds:%i tests:%i renders:%i\n",
				   backEnd.pc.c_flareAdds, backEnd.pc.c_flareTests, backEnd.pc.c_flareRenders );
	}

	memset( &tr.pc, 0, sizeof( tr.pc ) );
	memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
}


void R_InitCommandBuffers( void ) {
	glConfig.smpActive = qfalse;
}

void R_ShutdownCommandBuffers( void ) {
	// kill the rendering thread
	if ( glConfig.smpActive ) {
		GLimp_WakeRenderer( NULL );
		glConfig.smpActive = qfalse;
	}
}

void R_IssueRenderCommands( qboolean runPerformanceCounters ) {
}
void R_SyncRenderThread( void ) {
}
void *R_GetCommandBuffer( int bytes ) {
}
#if 0 // moved to backend.
void    R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs ) {
void    RE_SetColor( const float *rgba ) {
void RE_StretchPic( float x, float y, float w, float h,
void RE_StretchPicGradient( float x, float y, float w, float h,
#endif

/*
====================
RE_BeginFrame

If running in stereo, RE_BeginFrame will be called twice
for each RE_EndFrame
====================
*/
void RE_BeginFrame( stereoFrame_t stereoFrame ) {
	glState.finishCalled = qfalse;

	tr.frameCount++;
	tr.frameSceneNum = 0;
	if ( r_textureMode->modified ) {
		GL_TextureMode( r_textureMode->string );
		r_textureMode->modified = qfalse;
	}
	if ( r_gamma->modified ) {
		r_gamma->modified = qfalse;
		R_SetColorMappings();
	}
	if ( !r_ignoreGLErrors->integer ) {
		int err;

		if ( ( err = qglGetError() ) != GL_NO_ERROR ) {
			ri.Error( ERR_FATAL, "RE_BeginFrame() - glGetError() failed (0x%x)!\n", err );
		}
	}
}

/*
=============
RE_EndFrame

Returns the number of msec spent in the back end
=============
*/
void RE_EndFrame( int *frontEndMsec, int *backEndMsec ) {
	if ( !tr.registered ) {
		return;
	}
	RB_SwapBuffers();
	// initalise counters
	R_ToggleSmpFrame();
	if ( frontEndMsec ) {
		*frontEndMsec = tr.frontEndMsec;
	}
	tr.frontEndMsec = 0;
	if ( backEndMsec ) {
		*backEndMsec = backEnd.pc.msec;
	}
	backEnd.pc.msec = 0;
}

