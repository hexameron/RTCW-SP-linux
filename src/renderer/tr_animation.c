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


#include "tr_local.h"

/*

All bones should be an identity orientation to display the mesh exactly
as it is specified.

For all other frames, the bones represent the transformation from the
orientation of the bone in the base frame to the orientation in this
frame.

*/

static mdsBoneFrame_t bones[MDS_MAX_BONES];

//-----------------------------------------------------------------------------

static float ProjectRadius( float r, vec3_t location ) {
	float pr;
	float dist;
	float c;
	vec3_t p;
	float projected[4];

	c = DotProduct( tr.viewParms.or.axis[0], tr.viewParms.or.origin );
	dist = DotProduct( tr.viewParms.or.axis[0], location ) - c;

	if ( dist <= 0 ) {
		return 0;
	}

	p[0] = 0;
	p[1] = fabs( r );
	p[2] = -dist;

	projected[0] = p[0] * tr.viewParms.projectionMatrix[0] +
				   p[1] * tr.viewParms.projectionMatrix[4] +
				   p[2] * tr.viewParms.projectionMatrix[8] +
				   tr.viewParms.projectionMatrix[12];

	projected[1] = p[0] * tr.viewParms.projectionMatrix[1] +
				   p[1] * tr.viewParms.projectionMatrix[5] +
				   p[2] * tr.viewParms.projectionMatrix[9] +
				   tr.viewParms.projectionMatrix[13];

	projected[2] = p[0] * tr.viewParms.projectionMatrix[2] +
				   p[1] * tr.viewParms.projectionMatrix[6] +
				   p[2] * tr.viewParms.projectionMatrix[10] +
				   tr.viewParms.projectionMatrix[14];

	projected[3] = p[0] * tr.viewParms.projectionMatrix[3] +
				   p[1] * tr.viewParms.projectionMatrix[7] +
				   p[2] * tr.viewParms.projectionMatrix[11] +
				   tr.viewParms.projectionMatrix[15];


	pr = projected[1] / projected[3];

	if ( pr > 1.0f ) {
		pr = 1.0f;
	}

	return pr;
}

/*
=============
R_CullModel
=============
*/
static int R_CullModel( mdsHeader_t *header, trRefEntity_t *ent ) {
	vec3_t bounds[2];
	mdsFrame_t  *oldFrame, *newFrame;
	int i, frameSize;
	qboolean cullSphere;
	float radScale;

	cullSphere = qtrue;

	frameSize = (int) ( sizeof( mdsFrame_t ) - sizeof( mdsBoneFrameCompressed_t ) + header->numBones * sizeof( mdsBoneFrameCompressed_t ) );

	// compute frame pointers
	newFrame = ( mdsFrame_t * )( ( byte * ) header + header->ofsFrames + ent->e.frame * frameSize );
	oldFrame = ( mdsFrame_t * )( ( byte * ) header + header->ofsFrames + ent->e.oldframe * frameSize );

	radScale = 1.0f;

	if ( ent->e.nonNormalizedAxes ) {
		cullSphere = qfalse;    // by defalut, cull bounding sphere ONLY if this is not an upscaled entity

		// but allow the radius to be scaled if specified
//		if(ent->e.reFlags & REFLAG_SCALEDSPHERECULL) {
//			cullSphere = qtrue;
//			radScale = ent->e.radius;
//		}
	}

	if ( cullSphere ) {
		if ( ent->e.frame == ent->e.oldframe ) {
			switch ( R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius * radScale ) )
			{
			case CULL_OUT:
				tr.pc.c_sphere_cull_md3_out++;
				return CULL_OUT;

			case CULL_IN:
				tr.pc.c_sphere_cull_md3_in++;
				return CULL_IN;

			case CULL_CLIP:
				tr.pc.c_sphere_cull_md3_clip++;
				break;
			}
		} else
		{
			int sphereCull, sphereCullB;

			sphereCull  = R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius * radScale );
			if ( newFrame == oldFrame ) {
				sphereCullB = sphereCull;
			} else {
				sphereCullB = R_CullLocalPointAndRadius( oldFrame->localOrigin, oldFrame->radius * radScale );
			}

			if ( sphereCull == sphereCullB ) {
				if ( sphereCull == CULL_OUT ) {
					tr.pc.c_sphere_cull_md3_out++;
					return CULL_OUT;
				} else if ( sphereCull == CULL_IN )   {
					tr.pc.c_sphere_cull_md3_in++;
					return CULL_IN;
				} else
				{
					tr.pc.c_sphere_cull_md3_clip++;
				}
			}
		}
	}

	// calculate a bounding box in the current coordinate system
	for ( i = 0 ; i < 3 ; i++ ) {
		bounds[0][i] = oldFrame->bounds[0][i] < newFrame->bounds[0][i] ? oldFrame->bounds[0][i] : newFrame->bounds[0][i];
		bounds[1][i] = oldFrame->bounds[1][i] > newFrame->bounds[1][i] ? oldFrame->bounds[1][i] : newFrame->bounds[1][i];

		bounds[0][i] *= radScale;   //----(SA)	added
		bounds[1][i] *= radScale;   //----(SA)	added
	}

	switch ( R_CullLocalBox( bounds ) )
	{
	case CULL_IN:
		tr.pc.c_box_cull_md3_in++;
		return CULL_IN;
	case CULL_CLIP:
		tr.pc.c_box_cull_md3_clip++;
		return CULL_CLIP;
	case CULL_OUT:
	default:
		tr.pc.c_box_cull_md3_out++;
		return CULL_OUT;
	}
}

/*
=================
R_CalcMDSLod

=================
*/
float R_CalcMDSLod( refEntity_t *refent, vec3_t origin, float radius, float modelBias, float modelScale ) {
	float flod, lodScale;
	float projectedRadius;

	if ( refent->reFlags & REFLAG_FULL_LOD ) {
		return 1.0f;
	}

	// compute projected bounding sphere and use that as a criteria for selecting LOD

	projectedRadius = ProjectRadius( radius, origin );
	if ( projectedRadius != 0 ) {

//		ri.Printf (PRINT_ALL, "projected radius: %f\n", projectedRadius);

		lodScale = r_lodscale->value;   // fudge factor since MDS uses a much smoother method of LOD
		flod = projectedRadius * lodScale * modelScale;
	} else
	{
		// object intersects near view plane, e.g. view weapon
		flod = 1.0f;
	}

	if ( refent->reFlags & REFLAG_FORCE_LOD ) {
		flod *= 0.5;
	}
//----(SA)	like reflag_force_lod, but separate for the moment
	if ( refent->reFlags & REFLAG_DEAD_LOD ) {
		flod *= 0.8;
	}

	flod -= 0.25 * ( r_lodbias->value ) + modelBias;

	if ( flod < 0.0 ) {
		flod = 0.0;
	} else if ( flod > 1.0f ) {
		flod = 1.0f;
	}

	return flod;
}

/*
=================
R_ComputeFogNum

=================
*/
static int R_ComputeFogNum( mdsHeader_t *header, trRefEntity_t *ent ) {
	int i, j;
	fog_t           *fog;
	mdsFrame_t      *mdsFrame;
	vec3_t localOrigin;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}

	// FIXME: non-normalized axis issues
	mdsFrame = ( mdsFrame_t * )( ( byte * ) header + header->ofsFrames + ( sizeof( mdsFrame_t ) + sizeof( mdsBoneFrameCompressed_t ) * ( header->numBones - 1 ) ) * ent->e.frame );
	VectorAdd( ent->e.origin, mdsFrame->localOrigin, localOrigin );
	for ( i = 1 ; i < tr.world->numfogs ; i++ ) {
		fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( localOrigin[j] - mdsFrame->radius >= fog->bounds[1][j] ) {
				break;
			}
			if ( localOrigin[j] + mdsFrame->radius <= fog->bounds[0][j] ) {
				break;
			}
		}
		if ( j == 3 ) {
			return i;
		}
	}

	return 0;
}

/*
==============
R_AddAnimSurfaces
==============
*/
void R_AddAnimSurfaces( trRefEntity_t *ent ) {
	mdsHeader_t     *header;
	mdsSurface_t    *surface;
	shader_t        *shader = 0;
	int i, fogNum, cull;
	qboolean personalModel;

	// don't add third_person objects if not in a portal
	personalModel = ( ent->e.renderfx & RF_THIRD_PERSON ) && !tr.viewParms.isPortal;

	header = tr.currentModel->mds;

	//
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum.
	//
	cull = R_CullModel( header, ent );
	if ( cull == CULL_OUT ) {
		return;
	}

	//
	// set up lighting now that we know we aren't culled
	//
	if ( !personalModel || r_shadows->integer > 1 ) {
		R_SetupEntityLighting( &tr.refdef, ent );
	}

	//
	// see if we are in a fog volume
	//
	fogNum = R_ComputeFogNum( header, ent );

	surface = ( mdsSurface_t * )( (byte *)header + header->ofsSurfaces );
	for ( i = 0 ; i < header->numSurfaces ; i++ ) {
		int j;

//----(SA)	blink will change to be an overlay rather than replacing the head texture.
//		think of it like batman's mask.  the polygons that have eye texture are duplicated
//		and the 'lids' rendered with polygonoffset over the top of the open eyes.  this gives
//		minimal overdraw/alpha blending/texture use without breaking the model and causing seams
		if ( !Q_stricmp( surface->name, "h_blink" ) ) {
			if ( !( ent->e.renderfx & RF_BLINK ) ) {
				surface = ( mdsSurface_t * )( (byte *)surface + surface->ofsEnd );
				continue;
			}
		}
//----(SA)	end


		if ( ent->e.customShader ) {
			shader = R_GetShaderByHandle( ent->e.customShader );
		} else if ( ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins ) {
			skin_t *skin;

			skin = R_GetSkinByHandle( ent->e.customSkin );

			// match the surface name to something in the skin file
			shader = tr.defaultShader;
			for ( j = 0 ; j < skin->numSurfaces ; j++ ) {
				// the names have both been lowercased
				if ( !strcmp( skin->surfaces[j]->name, surface->name ) ) {
					shader = skin->surfaces[j]->shader;
					break;
				}
			}

			if ( shader == tr.defaultShader ) {
				ri.Printf( PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surface->name, skin->name );
			} else if ( shader->defaultShader )     {
				ri.Printf( PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name );
			}
		} else {
			shader = R_GetShaderByHandle( surface->shaderIndex );
		}

		// don't add third_person objects if not viewing through a portal
		if ( !personalModel ) {
			// GR - always tessellate these objects
			R_AddDrawSurf( (void *)surface, shader, fogNum, qfalse, ATI_TESS_TRUFORM );
		}

		surface = ( mdsSurface_t * )( (byte *)surface + surface->ofsEnd );
	}
}

__inline void LocalMatrixTransformVector( vec3_t in, vec3_t mat[ 3 ], vec3_t out ) {
	out[ 0 ] = in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ];
	out[ 1 ] = in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ];
	out[ 2 ] = in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ];
}

__inline void LocalMatrixTransformVectorTranslate( vec3_t in, vec3_t mat[ 3 ], vec3_t tr, vec3_t out ) {
	out[ 0 ] = in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] + tr[ 0 ];
	out[ 1 ] = in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] + tr[ 1 ];
	out[ 2 ] = in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] + tr[ 2 ];
}

__inline void LocalScaledMatrixTransformVector( vec3_t in, float s, vec3_t mat[ 3 ], vec3_t out ) {
	out[ 0 ] = ( 1.0f - s ) * in[ 0 ] + s * ( in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] );
	out[ 1 ] = ( 1.0f - s ) * in[ 1 ] + s * ( in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] );
	out[ 2 ] = ( 1.0f - s ) * in[ 2 ] + s * ( in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] );
}

__inline void LocalScaledMatrixTransformVectorTranslate( vec3_t in, float s, vec3_t mat[ 3 ], vec3_t tr, vec3_t out ) {
	out[ 0 ] = ( 1.0f - s ) * in[ 0 ] + s * ( in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] + tr[ 0 ] );
	out[ 1 ] = ( 1.0f - s ) * in[ 1 ] + s * ( in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] + tr[ 1 ] );
	out[ 2 ] = ( 1.0f - s ) * in[ 2 ] + s * ( in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] + tr[ 2 ] );
}

__inline void LocalScaledMatrixTransformVectorFullTranslate( vec3_t in, float s, vec3_t mat[ 3 ], vec3_t tr, vec3_t out ) {
	out[ 0 ] = ( 1.0f - s ) * in[ 0 ] + s * ( in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] ) + tr[ 0 ];
	out[ 1 ] = ( 1.0f - s ) * in[ 1 ] + s * ( in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] ) + tr[ 1 ];
	out[ 2 ] = ( 1.0f - s ) * in[ 2 ] + s * ( in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] ) + tr[ 2 ];
}

__inline void LocalAddScaledMatrixTransformVectorFullTranslate( vec3_t in, float s, vec3_t mat[ 3 ], vec3_t tr, vec3_t out ) {
	out[ 0 ] += s * ( in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] ) + tr[ 0 ];
	out[ 1 ] += s * ( in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] ) + tr[ 1 ];
	out[ 2 ] += s * ( in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] ) + tr[ 2 ];
}

__inline void LocalAddScaledMatrixTransformVectorTranslate( vec3_t in, float s, vec3_t mat[ 3 ], vec3_t tr, vec3_t out ) {
	out[ 0 ] += s * ( in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] + tr[ 0 ] );
	out[ 1 ] += s * ( in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] + tr[ 1 ] );
	out[ 2 ] += s * ( in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] + tr[ 2 ] );
}

__inline void LocalAddScaledMatrixTransformVector( vec3_t in, float s, vec3_t mat[ 3 ], vec3_t out ) {
	out[ 0 ] += s * ( in[ 0 ] * mat[ 0 ][ 0 ] + in[ 1 ] * mat[ 0 ][ 1 ] + in[ 2 ] * mat[ 0 ][ 2 ] );
	out[ 1 ] += s * ( in[ 0 ] * mat[ 1 ][ 0 ] + in[ 1 ] * mat[ 1 ][ 1 ] + in[ 2 ] * mat[ 1 ][ 2 ] );
	out[ 2 ] += s * ( in[ 0 ] * mat[ 2 ][ 0 ] + in[ 1 ] * mat[ 2 ][ 1 ] + in[ 2 ] * mat[ 2 ][ 2 ] );
}

static float LAVangle;
//static float		sr; // TTimo: unused
static float sp, sy;
//static float    cr; // TTimo: unused
static float cp, cy;

__inline void LocalAngleVector( vec3_t angles, vec3_t forward ) {
	LAVangle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( LAVangle );
	cy = cos( LAVangle );
	LAVangle = angles[PITCH] * ( M_PI * 2 / 360 );
	sp = sin( LAVangle );
	cp = cos( LAVangle );

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
}

__inline void LocalVectorMA( vec3_t org, float dist, vec3_t vec, vec3_t out ) {
	out[0] = org[0] + dist * vec[0];
	out[1] = org[1] + dist * vec[1];
	out[2] = org[2] + dist * vec[2];
}

#define ANGLES_SHORT_TO_FLOAT( pf, sh )     { *( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = SHORT2ANGLE( *( sh++ ) ); }

__inline void SLerp_Normal( vec3_t from, vec3_t to, float tt, vec3_t out ) {
	float ft = 1.0 - tt;

	out[0] = from[0] * ft + to[0] * tt;
	out[1] = from[1] * ft + to[1] * tt;
	out[2] = from[2] * ft + to[2] * tt;

	VectorNormalize( out );
}

/*
===============================================================================

4x4 Matrices

===============================================================================
*/

__inline void Matrix4Multiply( const vec4_t a[4], const vec4_t b[4], vec4_t dst[4] ) {
	dst[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
	dst[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
	dst[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
	dst[0][3] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];

	dst[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
	dst[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
	dst[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
	dst[1][3] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];

	dst[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
	dst[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
	dst[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
	dst[2][3] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];

	dst[3][0] = a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0];
	dst[3][1] = a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1];
	dst[3][2] = a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2];
	dst[3][3] = a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3];
}

// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c in Wolf MP source
__inline void Matrix4MultiplyInto3x3AndTranslation( /*const*/ vec4_t a[4], /*const*/ vec4_t b[4], vec3_t dst[3], vec3_t t ) {
	dst[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
	dst[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
	dst[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
	t[0]      = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];

	dst[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
	dst[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
	dst[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
	t[1]      = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];

	dst[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
	dst[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
	dst[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
	t[2]      = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];
}

__inline void Matrix4Transpose( const vec4_t matrix[4], vec4_t transpose[4] ) {
	int i, j;
	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

__inline void Matrix4FromAxis( const vec3_t axis[3], vec4_t dst[4] ) {
	int i, j;
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[i][j] = axis[i][j];
		}
		dst[3][i] = 0;
		dst[i][3] = 0;
	}
	dst[3][3] = 1;
}

__inline void Matrix4FromScaledAxis( const vec3_t axis[3], const float scale, vec4_t dst[4] ) {
	int i, j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[i][j] = scale * axis[i][j];
			if ( i == j ) {
				dst[i][j] += 1.0f - scale;
			}
		}
		dst[3][i] = 0;
		dst[i][3] = 0;
	}
	dst[3][3] = 1;
}

__inline void Matrix4FromTranslation( const vec3_t t, vec4_t dst[4] ) {
	int i, j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			if ( i == j ) {
				dst[i][j] = 1;
			} else {
				dst[i][j] = 0;
			}
		}
		dst[i][3] = t[i];
		dst[3][i] = 0;
	}
	dst[3][3] = 1;
}

// can put an axis rotation followed by a translation directly into one matrix
// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c in Wolf MP source
__inline void Matrix4FromAxisPlusTranslation( /*const*/ vec3_t axis[3], const vec3_t t, vec4_t dst[4] ) {
	int i, j;
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[i][j] = axis[i][j];
		}
		dst[3][i] = 0;
		dst[i][3] = t[i];
	}
	dst[3][3] = 1;
}

// can put a scaled axis rotation followed by a translation directly into one matrix
// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c in Wolf MP source
__inline void Matrix4FromScaledAxisPlusTranslation( /*const*/ vec3_t axis[3], const float scale, const vec3_t t, vec4_t dst[4] ) {
	int i, j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[i][j] = scale * axis[i][j];
			if ( i == j ) {
				dst[i][j] += 1.0f - scale;
			}
		}
		dst[3][i] = 0;
		dst[i][3] = t[i];
	}
	dst[3][3] = 1;
}

__inline void Matrix4FromScale( const float scale, vec4_t dst[4] ) {
	int i, j;

	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			if ( i == j ) {
				dst[i][j] = scale;
			} else {
				dst[i][j] = 0;
			}
		}
	}
	dst[3][3] = 1;
}

__inline void Matrix4TransformVector( const vec4_t m[4], const vec3_t src, vec3_t dst ) {
	dst[0] = m[0][0] * src[0] + m[0][1] * src[1] + m[0][2] * src[2] + m[0][3];
	dst[1] = m[1][0] * src[0] + m[1][1] * src[1] + m[1][2] * src[2] + m[1][3];
	dst[2] = m[2][0] * src[0] + m[2][1] * src[1] + m[2][2] * src[2] + m[2][3];
}

/*
===============================================================================

3x3 Matrices

===============================================================================
*/

__inline void Matrix3Transpose( const vec3_t matrix[3], vec3_t transpose[3] ) {
	int i, j;
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
==============
R_CalcBones

The list of bones[] should only be built and modified from within here
==============
*/
void R_CalcBones( mdsHeader_t *header, const refEntity_t *refent, int *boneList, int numBones ) {

	int   i, j;
	int   *boneRefs;
	float torsoWeight;
	mdsBoneFrame_t  *bonePtr, *parentBone;
	mdsFrame_t      *frame, *torsoFrame;
	mdsBoneInfo_t   *boneInfo, *thisBoneInfo, *parentBoneInfo;
	mdsBoneFrameCompressed_t    *cBonePtr, *cTBonePtr,  *cBoneList, *cBoneListTorso;
	vec3_t t, torsoAxis[3], tmpAxis[3], torsoParentOffset;
	vec4_t m1[4], m2[4];
	int frameSize;

	frameSize = (int) ( sizeof( mdsFrame_t ) + ( header->numBones - 1 ) * sizeof( mdsBoneFrameCompressed_t ) );
	frame = ( mdsFrame_t * )( (byte *)header + header->ofsFrames + refent->frame * frameSize );
	torsoFrame = ( mdsFrame_t * )( (byte *)header + header->ofsFrames + refent->torsoFrame * frameSize );

	boneInfo = ( mdsBoneInfo_t * )( (byte *)header + header->ofsBones );
	boneRefs = boneList;
	Matrix3Transpose( refent->torsoAxis, torsoAxis );
	cBoneList = frame->bones;
	cBoneListTorso = torsoFrame->bones;

	for ( i = 0; i < numBones; i++, boneRefs++ ) {
	// R_CalcBone( header, refent, *boneRefs );
//void R_CalcBone( mdsHeader_t *header, const refEntity_t *refent, int boneNum ) {
{
	int     boneNum;
	short	*sh;
	float	*pf, diff;
	vec3_t tangles, angles, vec, v2;
	qboolean isTorso, fullTorso;

	boneNum = *boneRefs;
	thisBoneInfo = &boneInfo[boneNum];
	if ( thisBoneInfo->torsoWeight ) {
		cTBonePtr = &cBoneListTorso[boneNum];
		isTorso = qtrue;
		if ( thisBoneInfo->torsoWeight == 1.0f ) {
			fullTorso = qtrue;
		}
	} else {
		isTorso = qfalse;
		fullTorso = qfalse;
	}
	cBonePtr = &cBoneList[boneNum];

	bonePtr = &bones[ boneNum ];

	// we can assume the parent has already been uncompressed for this frame + lerp
	if ( thisBoneInfo->parent >= 0 ) {
		parentBone = &bones[ thisBoneInfo->parent ];
		parentBoneInfo = &boneInfo[ thisBoneInfo->parent ];
	} else {
		parentBone = NULL;
		parentBoneInfo = NULL;
	}

	// rotation
	if ( fullTorso ) {
		sh = (short *)cTBonePtr->angles;
		pf = angles;
		ANGLES_SHORT_TO_FLOAT( pf, sh );
	} else {
		sh = (short *)cBonePtr->angles;
		pf = angles;
		ANGLES_SHORT_TO_FLOAT( pf, sh );
		if ( isTorso ) {
			sh = (short *)cTBonePtr->angles;
			pf = tangles;
			ANGLES_SHORT_TO_FLOAT( pf, sh );
			// blend the angles together
			for ( j = 0; j < 3; j++ ) {
				diff = tangles[j] - angles[j];
				if ( fabs( diff ) > 180 ) {
					diff = AngleNormalize180( diff );
				}
				angles[j] = angles[j] + thisBoneInfo->torsoWeight * diff;
			}
		}
	}
	AnglesToAxis( angles, bonePtr->matrix );

	// translation
	if ( parentBone ) {

		if ( fullTorso ) {
			sh = (short *)cTBonePtr->ofsAngles; pf = angles;
			*( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = 0;
			LocalAngleVector( angles, vec );
			LocalVectorMA( parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation );
		} else {

			sh = (short *)cBonePtr->ofsAngles; pf = angles;
			*( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = 0;
			LocalAngleVector( angles, vec );

			if ( isTorso ) {
				sh = (short *)cTBonePtr->ofsAngles;
				pf = tangles;
				*( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = SHORT2ANGLE( *( sh++ ) ); *( pf++ ) = 0;
				LocalAngleVector( tangles, v2 );

				// blend the angles together
				SLerp_Normal( vec, v2, thisBoneInfo->torsoWeight, vec );
				LocalVectorMA( parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation );

			} else {    // legs bone
				LocalVectorMA( parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation );
			}
		}
	} else {    // just use the frame position
		bonePtr->translation[0] = frame->parentOffset[0];
		bonePtr->translation[1] = frame->parentOffset[1];
		bonePtr->translation[2] = frame->parentOffset[2];
	}
	//
	if ( boneNum == header->torsoParent ) { // this is the torsoParent
		VectorCopy( bonePtr->translation, torsoParentOffset );
	}
}
// } void R_CalcBone( mdsHeader_t *header, const refEntity_t *refent, int boneNum )
	}

	// adjust for torso rotations
	torsoWeight = 0;
	boneRefs = boneList;
	for ( i = 0; i < numBones; i++, boneRefs++ ) {

		thisBoneInfo = &boneInfo[ *boneRefs ];
		bonePtr = &bones[ *boneRefs ];
		// add torso rotation
		if ( thisBoneInfo->torsoWeight > 0 ) {

			if ( !( thisBoneInfo->flags & BONEFLAG_TAG ) ) {

				// 1st multiply with the bone->matrix
				// 2nd translation for rotation relative to bone around torso parent offset
				VectorSubtract( bonePtr->translation, torsoParentOffset, t );
				Matrix4FromAxisPlusTranslation( bonePtr->matrix, t, m1 );
				// 3rd scaled rotation
				// 4th translate back to torso parent offset
				// use previously created matrix if available for the same weight
				if ( torsoWeight != thisBoneInfo->torsoWeight ) {
					Matrix4FromScaledAxisPlusTranslation( torsoAxis, thisBoneInfo->torsoWeight, torsoParentOffset, m2 );
					torsoWeight = thisBoneInfo->torsoWeight;
				}
				// multiply matrices to create one matrix to do all calculations
				Matrix4MultiplyInto3x3AndTranslation( m2, m1, bonePtr->matrix, bonePtr->translation );

			} else {    // tag's require special handling

				// rotate each of the axis by the torsoAngles
				LocalScaledMatrixTransformVector( bonePtr->matrix[0], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[0] );
				LocalScaledMatrixTransformVector( bonePtr->matrix[1], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[1] );
				LocalScaledMatrixTransformVector( bonePtr->matrix[2], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[2] );
				memcpy( bonePtr->matrix, tmpAxis, sizeof( tmpAxis ) );

				// rotate the translation around the torsoParent
				VectorSubtract( bonePtr->translation, torsoParentOffset, t );
				LocalScaledMatrixTransformVector( t, thisBoneInfo->torsoWeight, torsoAxis, bonePtr->translation );
				VectorAdd( bonePtr->translation, torsoParentOffset, bonePtr->translation );

			}
		}
	}
}

#ifdef DBG_PROFILE_BONES
#define DBG_SHOWTIME    Com_Printf( "%i: %i, ", di++, ( dt = ri.Milliseconds() ) - ldt ); ldt = dt;
#else
#define DBG_SHOWTIME    ;
#endif

/*
==============
RB_SurfaceAnim
==============
*/
void RB_SurfaceAnim( mdsSurface_t *surface ) {
	int i, j, k;
	int render_count;
	int p0, p1, p2, indexes;
	refEntity_t     *refent;
	int             *boneList;
	int *triangles, *pIndexes;
	mdsHeader_t     *header;
	mdsFrame_t      *backframe;
	mdsBoneFrame_t  *bone;
	mdsVertex_t     *v;
	vec3_t          vec;
	float lodRadius, lodScale;
	float *tempVert, *tempNormal;
	int   baseIndex, baseVertex, oldIndexes, numVerts;
	int   frameSize;
	int *collapse_map, *pCollapseMap;
	int collapse[ MDS_MAX_VERTS ], *pCollapse;

	refent = &backEnd.currentEntity->e;
	boneList = ( int * )( (byte *)surface + surface->ofsBoneReferences );
	header = ( mdsHeader_t * )( (byte *)surface + surface->ofsHeader );
	frameSize = (int) ( sizeof( mdsFrame_t ) + ( header->numBones - 1 ) * sizeof( mdsBoneFrameCompressed_t ) );
	backframe = ( mdsFrame_t * )( (byte *)header + header->ofsFrames + refent->frame * frameSize );
	//
	// calculate LOD
	//
	// TODO: lerp the radius and origin
	VectorAdd( refent->origin, backframe->localOrigin, vec );
	lodRadius = backframe->radius;
	lodScale = R_CalcMDSLod( refent, vec, lodRadius, header->lodBias, header->lodScale );

//----(SA)	modification to allow dead skeletal bodies to go below minlod (experiment)
	if ( refent->reFlags & REFLAG_DEAD_LOD ) {
		if ( lodScale < 0.35 ) {   // allow dead to lod down to 35% (even if below surf->minLod) (%35 is arbitrary and probably not good generally.  worked for the blackguard/infantry as a test though)
			lodScale = 0.35;
		}
		render_count = (int)( (float) surface->numVerts * lodScale );

	} else {
		render_count = (int)( (float) surface->numVerts * lodScale );
		if ( render_count < surface->minLod ) {
			if ( !( refent->reFlags & REFLAG_DEAD_LOD ) ) {
				render_count = surface->minLod;
			}
		}
	}
//----(SA)	end

	if ( render_count > surface->numVerts ) {
		render_count = surface->numVerts;
	}

	RB_CheckOverflow( render_count, surface->numTriangles );

	//
	// setup triangle list
	//
	RB_CheckOverflow( surface->numVerts, surface->numTriangles * 3 );

	collapse_map   = ( int * )( ( byte * )surface + surface->ofsCollapseMap );
	triangles = ( int * )( (byte *)surface + surface->ofsTriangles );
	indexes = surface->numTriangles * 3;
	baseIndex = tess.numIndexes;
	baseVertex = tess.numVertexes;
	oldIndexes = baseIndex;

	tess.numVertexes += render_count;

	pIndexes = &tess.indexes[baseIndex];

	if ( render_count == surface->numVerts ) {
		memcpy( pIndexes, triangles, sizeof( triangles[0] ) * indexes );
		if ( baseVertex ) {
			int *indexesEnd;
			for ( indexesEnd = pIndexes + indexes ; pIndexes < indexesEnd ; pIndexes++ ) {
				*pIndexes += baseVertex;
			}
		}
		tess.numIndexes += indexes;
	} else
	{
		int *collapseEnd;

		pCollapse = collapse;
		for ( j = 0; j < render_count; pCollapse++, j++ )
		{
			*pCollapse = j;
		}

		pCollapseMap = &collapse_map[render_count];
		for ( collapseEnd = collapse + surface->numVerts ; pCollapse < collapseEnd; pCollapse++, pCollapseMap++ )
		{
			*pCollapse = collapse[ *pCollapseMap ];
		}

		for ( j = 0 ; j < indexes ; j += 3 )
		{
			p0 = collapse[ *( triangles++ ) ];
			p1 = collapse[ *( triangles++ ) ];
			p2 = collapse[ *( triangles++ ) ];

			// FIXME
			// note:  serious optimization opportunity here,
			//  by sorting the triangles the following "continue"
			//  could have been made into a "break" statement.
			if ( p0 == p1 || p1 == p2 || p2 == p0 ) {
				continue;
			}

			*( pIndexes++ ) = baseVertex + p0;
			*( pIndexes++ ) = baseVertex + p1;
			*( pIndexes++ ) = baseVertex + p2;
			tess.numIndexes += 3;
		}

		baseIndex = tess.numIndexes;
	}

	//
	// deform the vertexes by the lerped bones
	//
	numVerts = surface->numVerts;
	v = ( mdsVertex_t * )( (byte *)surface + surface->ofsVerts );
	tempVert = ( float * )( tess.xyz + baseVertex );
	tempNormal = ( float * )( tess.normal + baseVertex );

	GLimp_LockBones( qtrue );
	R_CalcBones( header, (const refEntity_t *)refent, boneList, surface->numBoneReferences );

	for ( j = 0; j < render_count; j++, tempVert += 4, tempNormal += 4 ) {
		mdsWeight_t *w;

		VectorClear( tempVert );

		w = v->weights;
		for ( k = 0 ; k < v->numWeights ; k++, w++ ) {
			bone = &bones[w->boneIndex];
			LocalAddScaledMatrixTransformVectorTranslate( w->offset, w->boneWeight, bone->matrix, bone->translation, tempVert );
		}
		LocalMatrixTransformVector( v->normal, bones[v->weights[0].boneIndex].matrix, tempNormal );

		tess.texCoords[baseVertex + j][0][0] = v->texCoords[0];
		tess.texCoords[baseVertex + j][0][1] = v->texCoords[1];

		v = (mdsVertex_t *)&v->weights[v->numWeights];
	}
	GLimp_LockBones( qfalse );
}

/*
===============
R_RecursiveBoneListAdd
===============
*/
void R_RecursiveBoneListAdd( int bi, int *boneList, int *numBones, mdsBoneInfo_t *boneInfoList ) {

	if ( boneInfoList[ bi ].parent >= 0 ) {

		R_RecursiveBoneListAdd( boneInfoList[ bi ].parent, boneList, numBones, boneInfoList );

	}

	boneList[ ( *numBones )++ ] = bi;

}

/*
===============
R_GetBoneTag
===============
*/
int R_GetBoneTag( orientation_t *outTag, mdsHeader_t *mds, int startTagIndex, const refEntity_t *refent, const char *tagName ) {

	int i;
	mdsTag_t    *pTag;
	mdsBoneInfo_t *boneInfoList;
	int boneList[ MDS_MAX_BONES ];
	int numBones;

	if ( startTagIndex > mds->numTags ) {
		memset( outTag, 0, sizeof( *outTag ) );
		return -1;
	}

	// find the correct tag

	pTag = ( mdsTag_t * )( (byte *)mds + mds->ofsTags );

	pTag += startTagIndex;

	for ( i = startTagIndex; i < mds->numTags; i++, pTag++ ) {
		if ( !strcmp( pTag->name, tagName ) ) {
			break;
		}
	}

	if ( i >= mds->numTags ) {
		memset( outTag, 0, sizeof( *outTag ) );
		return -1;
	}

	// now build the list of bones we need to calc to get this tag's bone information

	boneInfoList = ( mdsBoneInfo_t * )( (byte *)mds + mds->ofsBones );
	numBones = 0;

	GLimp_LockBones( qtrue );

	R_RecursiveBoneListAdd( pTag->boneIndex, boneList, &numBones, boneInfoList );
	R_CalcBones( (mdsHeader_t *)mds, refent, boneList, numBones );

	GLimp_LockBones( qfalse );

	// now extract the orientation for the bone that represents our tag

	memcpy( outTag->axis, bones[ pTag->boneIndex ].matrix, sizeof( outTag->axis ) );
	VectorCopy( bones[ pTag->boneIndex ].translation, outTag->origin );

	return i;
}
