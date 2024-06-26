/*
 * Wolfenstein: Enemy Territory GPL Source Code
 * Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
 * Copyright (C) 2010-2011 Robert Beckebans <trebor_7@users.sourceforge.net>
 *
 * ET: Legacy
 * Copyright (C) 2012-2024 ET:Legacy team <mail@etlegacy.com>
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
 */
/**
 * @file renderer2/tr_animation_mdm.c
 *
 * @brief All bones should be an identity orientation to display the mesh exactly
 * as it is specified.
 *
 * For all other frames, the bones represent the transformation from the
 * orientation of the bone in the base frame to the orientation in this
 * frame.
 */

#include "tr_local.h"

#pragma warning(disable:4700)
/*
	An effort was done to get rid of many function-calls.
	Most calculational functions have been inlined already (in q_math, tr_extra ...)
	Because playermodel rendering appeared to be a demanding task for ET, this file got some special attention.
	There were some functions that peaked in cpu-usage, such as GetMDMSurfaceShader() and R_CalcBones(),
	and for that code also some logic flow changes were done too. (see references to 'personalModel'.. pfff).
*/
// undef to use floating-point lerping with explicit trig funcs
#define YD_INGLES

//#define DBG_PROFILE_BONES

//-----------------------------------------------------------------------------
// Static Vars, ugly but easiest (and fastest) means of seperating RB_SurfaceAnim
// and R_CalcBones

static mdxBoneFrame_t           bones[MDX_MAX_BONES], rawBones[MDX_MAX_BONES], oldBones[MDX_MAX_BONES];
static char                     validBones[MDX_MAX_BONES];
static char                     newBones[MDX_MAX_BONES];
static mdxBoneFrameCompressed_t *cBoneList, *cOldBoneList, *cBoneListTorso, *cOldBoneListTorso;
static mdxBoneInfo_t            *boneInfo, *thisBoneInfo, *parentBoneInfo;
static mdxFrame_t               *frame, *torsoFrame, *oldFrame, *oldTorsoFrame;
static vec3_t                   torsoParentOffset;
static refEntity_t              lastBoneEntity;
static int                      totalrv, totalrt, totalv, totalt;

/// All the following static vars have been made function-local
//static int                      *boneRefs; // unused
//static int                      p0, p1, p2; // unused
//static float                    diff, a1, a2;
//static float                    *pf;
//static short                    *sh, *sh2;
//static vec3_t                   vec, v2, dir;
//static vec3_t                   t;
//static vec4_t                   m1[4], m2[4];
//static qboolean                 isTorso, fullTorso;
//static float                    lodRadius, lodScale;
//static int                      render_count;
//static float                    frontlerp, backlerp, torsoFrontlerp, torsoBacklerp;
//static int                      frameSize;
//static int32_t                  collapse[MDM_MAX_VERTS], *pCollapse, *pCollapseMap;
//static mdxBoneFrameCompressed_t *cBonePtr, *cTBonePtr, *cOldBonePtr, *cOldTBonePtr;
//static mdxBoneFrame_t           *bonePtr, *bone, *parentBone;
//static vec3_t                   torsoAxis[3];
//static int                      ingles[3], tingles[3];
//static vec3_t                   angles, tangles;

//-----------------------------------------------------------------------------

/**
 * @brief RB_ProjectRadius
 * @param[in] r
 * @param[in] location
 * @return
 */
static float RB_ProjectRadius(float r, vec3_t location)
{
	float  pr;
	float  dist;
	float  c;
	vec4_t p;
	float  projected[4];

	Dot(backEnd.viewParms.orientation.axis[0], backEnd.viewParms.orientation.origin, c);
	Dot(backEnd.viewParms.orientation.axis[0], location, dist);
	dist -= c;

	if (dist <= 0)
	{
		return 0;
	}

	p[0] = 0;
	p[1] = Q_fabs(r);
	p[2] = -dist;
	p[3] = 1.0; // vec4

	Vector4TransformM4(backEnd.viewParms.projectionMatrix, p, projected);

	pr = projected[1] / projected[3];

	if (pr > 1.0f)
	{
		pr = 1.0f;
	}

	return pr;
}

/**
 * @brief R_CullModel
 * @param[in,out] ent
 * @return
 */
static int R_CullModel(trRefEntity_t *ent)
{
	mdxHeader_t *oldFrameHeader, *newFrameHeader;
	mdxFrame_t  *oldFrame, *newFrame;

	newFrameHeader = R_GetModelByHandle(ent->e.frameModel)->mdx;
	oldFrameHeader = R_GetModelByHandle(ent->e.oldframeModel)->mdx;

	if (!newFrameHeader || !oldFrameHeader)
	{
		return CULL_OUT;
	}

	// compute frame pointers
	newFrame = (mdxFrame_t *) ((byte *) newFrameHeader + newFrameHeader->ofsFrames +
	                           ent->e.frame * (int)(sizeof(mdxBoneFrameCompressed_t)) * newFrameHeader->numBones +
	                           ent->e.frame * sizeof(mdxFrame_t));
	oldFrame = (mdxFrame_t *) ((byte *) oldFrameHeader + oldFrameHeader->ofsFrames +
	                           ent->e.oldframe * (int)(sizeof(mdxBoneFrameCompressed_t)) * oldFrameHeader->numBones +
	                           ent->e.oldframe * sizeof(mdxFrame_t));

	// calculate a bounding box in the current coordinate system
	VectorMin(oldFrame->bounds[0], newFrame->bounds[0], ent->localBounds[0]);
	VectorMax(oldFrame->bounds[1], newFrame->bounds[1], ent->localBounds[1]);

	// setup world bounds for intersection tests
	R_SetupEntityWorldBounds(ent);

	// cull bounding sphere ONLY if this is not an upscaled entity
	if (!ent->e.nonNormalizedAxes)
	{
		// info: this code is executed (so playermodels appear to be not scaled)
		if (ent->e.frame == ent->e.oldframe && ent->e.frameModel == ent->e.oldframeModel)
		{
			switch (R_CullLocalPointAndRadius(newFrame->localOrigin, newFrame->radius))
			{
			case CULL_OUT:
				tr.pc.c_sphere_cull_mdx_out++;
				return CULL_OUT;
			case CULL_IN:
				tr.pc.c_sphere_cull_mdx_in++;
				return CULL_IN;
			case CULL_CLIP:
				tr.pc.c_sphere_cull_mdx_clip++;
				break;
			}
		}
		else
		{
			int sphereCull, sphereCullB;

			sphereCull = R_CullLocalPointAndRadius(newFrame->localOrigin, newFrame->radius);
			if (newFrame == oldFrame)
			{
				sphereCullB = sphereCull;
			}
			else
			{
				sphereCullB = R_CullLocalPointAndRadius(oldFrame->localOrigin, oldFrame->radius);
			}

			if (sphereCull == sphereCullB)
			{
				if (sphereCull == CULL_OUT)
				{
					tr.pc.c_sphere_cull_mdx_out++;
					return CULL_OUT;
				}
				else if (sphereCull == CULL_IN)
				{
					tr.pc.c_sphere_cull_mdx_in++;
					return CULL_IN;
				}
				else
				{
					tr.pc.c_sphere_cull_mdx_clip++;
				}
			}
		}
	}

	switch (R_CullLocalBox(ent->localBounds))
	{
	case CULL_IN:
		tr.pc.c_box_cull_mdx_in++;
		return CULL_IN;

	case CULL_CLIP:
		tr.pc.c_box_cull_mdx_clip++;
		return CULL_CLIP;

	case CULL_OUT:
	default:
		tr.pc.c_box_cull_mdx_out++;
		return CULL_OUT;
	}
}

/**
 * @brief RB_CalcMDMLod
 * @param[in] refent
 * @param[in] origin
 * @param[in] radius
 * @param[in] modelBias
 * @param[in] modelScale
 * @return
 */
static float RB_CalcMDMLod(refEntity_t *refent, vec3_t origin, float radius, float modelBias, float modelScale)
{
	float flod;
	float projectedRadius;

	// compute projected bounding sphere and use that as a criteria for selecting LOD

	projectedRadius = RB_ProjectRadius(radius, origin);
	if (projectedRadius != 0.f)
	{
		// ri.Printf (PRINT_ALL, "projected radius: %f\n", projectedRadius);
		flod = projectedRadius * r_lodScale->value * modelScale;
	}
	else
	{
		// object intersects near view plane, e.g. view weapon
		flod = 1.0f;
	}

	if (refent->reFlags & REFLAG_FORCE_LOD)
	{
		flod *= 0.5f;
	}
	// like reflag_force_lod, but separate for the moment
	if (refent->reFlags & REFLAG_DEAD_LOD)
	{
		flod *= 0.8f;
	}

	flod -= 0.25f * (r_lodBias->value) + modelBias;

	if (flod < 0.0f)
	{
		flod = 0.0f;
	}
	else if (flod > 1.0f)
	{
		flod = 1.0f;
	}

	return flod;
}

/**
 * @brief RB_CalcMDMLodIndex
 * @param[in] ent
 * @param[in] origin
 * @param[in] radius
 * @param[in] modelBias
 * @param[in] modelScale
 * @param mdmSurface - unused
 * @return
 */
static int RB_CalcMDMLodIndex(refEntity_t *ent, vec3_t origin, float radius, float modelBias, float modelScale, mdmSurfaceIntern_t *mdmSurface)
{
	float flod;
	int   lod;

	flod = RB_CalcMDMLod(ent, origin, radius, modelBias, modelScale);

	//flod = r_lodTest->value;

	// allow dead skeletal bodies to go below minlod (experiment)
#if 0
	if (ent->reFlags & REFLAG_DEAD_LOD)
	{
		if (flod < 0.35f)
		{
			// allow dead to lod down to 35% (even if below surf->minLod) (%35 is arbitrary and probably not good generally.
			// worked for the blackguard/infantry as a test though)
			flod = 0.35f;
		}
	}
	else
	{
		int render_count = round((float)mdmSurface->numVerts * flod);
		if (render_count < mdmSurface->minLod)
		{
			if (!(ent->reFlags & REFLAG_DEAD_LOD))
			{
				flod = (float)mdmSurface->minLod / mdmSurface->numVerts;
			}
		}
	}
#endif

	//for(lod = 0; lod < MD3_MAX_LODS; lod++)
	for (lod = MD3_MAX_LODS - 1; lod >= 0; lod--)
	{
		if (flod <= mdmLODResolutions[lod])
		{
			break;
		}
	}

	if (lod < 0)
	{
		lod = 0;
	}

	return lod;
}

/*
 * @brief R_ComputeFogNum
 * @param[in] ent
 * @return
 *
 * @note Unused
static int R_ComputeFogNum(trRefEntity_t * ent)
{
    int             i, j;
    fog_t          *fog;
    mdxHeader_t    *header;
    mdxFrame_t     *mdxFrame;
    vec3_t          localOrigin;

    if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
    {
        return 0;
    }

    header = R_GetModelByHandle(ent->e.frameModel)->mdx;

    // compute frame pointers
    mdxFrame = (mdxFrame_t *) ((byte *) header + header->ofsFrames +
                               ent->e.frame * (int)(sizeof(mdxBoneFrameCompressed_t)) * header->numBones +
                               ent->e.frame * sizeof(mdxFrame_t));

    // FIXME: non-normalized axis issues
    VectorAdd(ent->e.origin, mdxFrame->localOrigin, localOrigin);
    for(i = 1; i < tr.world->numfogs; i++)
    {
        fog = &tr.world->fogs[i];
        for(j = 0; j < 3; j++)
        {
            if(localOrigin[j] - mdxFrame->radius >= fog->bounds[1][j])
            {
                break;
            }
            if(localOrigin[j] + mdxFrame->radius <= fog->bounds[0][j])
            {
                break;
            }
        }
        if(j == 3)
        {
            return i;
        }
    }

    return 0;
}
*/

/**
 * @brief GetMDMSurfaceShader
 * @param[in] ent
 * @param[in] mdmSurface
 * @return
 *
 * @todo FIXME: cleanup ?
 */
static shader_t *GetMDMSurfaceShader(const trRefEntity_t *ent, mdmSurfaceIntern_t *mdmSurface)
{
	shader_t *shader = NULL;

	if (ent->e.customShader)
	{
		shader = R_GetShaderByHandle(ent->e.customShader);
	}
	else if (ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins)
	{
		skin_t *skin;
		int    j;

		skin = R_GetSkinByHandle(ent->e.customSkin);

		// match the surface name to something in the skin file
		shader = tr.defaultShader;

#if 1
#ifndef ETL_SSE
		// Q3A way
		for (j = 0; j < skin->numSurfaces; j++)
		{
			// the names have both been lowercased
			if (!strcmp(skin->surfaces[j].name, mdmSurface->name))
			{
				shader = skin->surfaces[j].shader;
				break;
			}
		}
#else
		// ^^that^^ strcmp is comparing two char[64]
		// SSE3 can access 16 bytes at once => that's just 4 reads needed to get the full 64 chars.
		// SSE can also compare 16 bytes at once..
		// But we need to check chars up to any found trailing 0 byte (that marks the end of the string)
		// If both strings are initialized before use, with 0, we can compare the whole strings very fast.
		// Another improvement is that we read mdmSurface->name only once (instead of repeatedly in the loop).
		// UPDATE: After testing this new SSE3 code, i am surprised about the performance gain.
		//         (Playermodels have just a few surfaces. Perhaps get rid of the loop totally?)
		__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm7, zeroes;
		int     mask0, mask16, mask32;
		zeroes = _mm_setzero_si128();
		xmm0   = _mm_loadu_si128((const __m128i *)&mdmSurface->name[0]);
		// find out if there's a 0-byte in this chunk
		// if there is no 0-byte, no bit is set in the mask
		mask0 = _mm_movemask_epi8(_mm_cmpeq_epi8(xmm0, zeroes));
		// we only need to read the remaining 3 chunks, if the 0-byte is not in the first chunk..
		// ..same for the rest of the chunks.
		// If we get to the last chunk, we don't need to test for the 0-byte anymore.
		if (mask0 == 0)
		{
			xmm1   = _mm_loadu_si128((const __m128i *)&mdmSurface->name[16]);
			mask16 = _mm_movemask_epi8(_mm_cmpeq_epi8(xmm1, zeroes));
			if (mask16 == 0)
			{
				xmm2   = _mm_loadu_si128((const __m128i *)&mdmSurface->name[32]);
				mask32 = _mm_movemask_epi8(_mm_cmpeq_epi8(xmm2, zeroes));
				if (mask32 == 0)
				{
					xmm3 = _mm_loadu_si128((const __m128i *)&mdmSurface->name[48]);
				}
			}
		}
		// Note that all surfacenames of a playermodel have names that do not exceed 16 bytes length,
		// so all the comparisons are done using just the first chunk.
		for (j = 0; j < skin->numSurfaces; j++)
		{
			xmm4 = _mm_loadu_si128((const __m128i *)&skin->surfaces[j].name[0]);
			// if unequal then continue
			xmm7 = _mm_cmpeq_epi8(xmm4, xmm0);  // there seems to be only an is-equal test for integers..
			// to test for 'is not equal', we must check if any of the returned (lowest) 16 bits are unset.
			// If the 16 bits are all set, the parts are equal.
			if (_mm_movemask_epi8(xmm7) != 0xFFFF)
			{
				continue;
			}
			// 16 bytes are equal. Now check if this was the last chunk
			// if there is a 0-byte in the chunk, some bit in mask0 is set
			if (mask0 != 0)
			{
				goto foundEqualStrings;
			}

			// else continue with next string parts
			xmm4 = _mm_loadu_si128((const __m128i *)&skin->surfaces[j].name[16]);
			xmm7 = _mm_cmpeq_epi8(xmm4, xmm1);
			if (_mm_movemask_epi8(xmm7) != 0xFFFF)
			{
				continue;
			}
			if (mask16 != 0)
			{
				goto foundEqualStrings;
			}

			xmm4 = _mm_loadu_si128((const __m128i *)&skin->surfaces[j].name[32]);
			xmm7 = _mm_cmpeq_epi8(xmm4, xmm2);
			if (_mm_movemask_epi8(xmm7) != 0xFFFF)
			{
				continue;
			}
			if (mask32 != 0)
			{
				goto foundEqualStrings;
			}

			xmm4 = _mm_loadu_si128((const __m128i *)&skin->surfaces[j].name[48]);
			xmm7 = _mm_cmpeq_epi8(xmm4, xmm3);
			if (_mm_movemask_epi8(xmm7) == 0xFFFF)
			{
				goto foundEqualStrings;
			}
		}

foundEqualStrings:
		if (j < skin->numSurfaces)
		{
			shader = skin->surfaces[j].shader;
		}
		else
		// !!! BEWARE that^^ else statement.
		// execute the next if-statement:  if (shader == tr.defaultShader)
#endif
#else
		if (ent->e.renderfx & RF_BLINK)
		{
			char *s = va("%s_b", surface->name);            // append '_b' for 'blink'

			hash = Com_HashKey(s, strlen(s));
			for (j = 0; j < skin->numSurfaces; j++)
			{
				if (hash != skin->surfaces[j].hash)
				{
					continue;
				}
				if (!strcmp(skin->surfaces[j].name, s))
				{
					shader = skin->surfaces[j].shader;
					break;
				}
			}
		}

		if (shader == tr.defaultShader)
		{
			// blink reference in skin was not found
			hash = Com_HashKey(surface->name, sizeof(surface->name));
			for (j = 0; j < skin->numSurfaces; j++)
			{
				// the names have both been lowercased
				if (hash != skin->surfaces[j].hash)
				{
					continue;
				}
				if (!strcmp(skin->surfaces[j].name, surface->name))
				{
					shader = skin->surfaces[j].shader;
					break;
				}
			}
		}
#endif

		if (shader == tr.defaultShader)
		{
			Ren_Developer("GetMDMSurfaceShader WARNING: no shader for surface %s in skin %s\n", mdmSurface->name, skin->name);
		}
		else if (shader->defaultShader)
		{
			Ren_Developer("GetMDMSurfaceShader WARNING: shader %s in skin %s not found\n", shader->name, skin->name);
		}
	}
	else
	{
		shader = R_GetShaderByHandle(mdmSurface->shaderIndex);
	}

	return shader;
}

/**
 * @brief R_MDM_AddAnimSurfaces
 * @param[in,out] ent
 */
void R_MDM_AddAnimSurfaces(trRefEntity_t *ent)
{
	mdmModel_t         *mdm = tr.currentModel->mdm;
	mdmSurfaceIntern_t *mdmSurface;
	shader_t           *shader = 0;
	int                i, fogNum;

	// don't add third_person objects if not in a portal
	if ((ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal)
	{
		return;
	}

	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum.
	ent->cull = (cullResult_t)(R_CullModel(ent));
	if (ent->cull == CULL_OUT)
	{
		return;
	}

	// set up lighting now that we know we aren't culled
	if (r_shadows->integer > SHADOWING_BLOB)
	{
		R_SetupEntityLighting(&tr.refdef, ent, NULL);
	}

	// see if we are in a fog volume
	fogNum = R_FogWorldBox(ent->worldBounds);

	// draw all surfaces
	if (r_vboModels->integer && mdm->numVBOSurfaces && glConfig2.vboVertexSkinningAvailable) // && ent->e.skeleton.type == SK_ABSOLUTE))
	{
		int             i;
		srfVBOMDMMesh_t *vboSurface;
		shader_t        *shader;

		for (i = 0; i < mdm->numVBOSurfaces; i++)
		{
			vboSurface = mdm->vboSurfaces[i];
			mdmSurface = vboSurface->mdmSurface;

			shader = GetMDMSurfaceShader(ent, mdmSurface);
			R_AddDrawSurf((surfaceType_t *)vboSurface, shader, LIGHTMAP_NONE, fogNum);
		}
	}
	else
	{
		for (i = 0, mdmSurface = mdm->surfaces; i < mdm->numSurfaces; i++, mdmSurface++)
		{
			shader = GetMDMSurfaceShader(ent, mdmSurface);
			R_AddDrawSurf((surfaceType_t *)mdmSurface, shader, LIGHTMAP_NONE, fogNum);
		}
	}
}

/**
 * @brief R_AddMDMInteractions
 * @param[in] ent
 * @param[in] light
 */
void R_AddMDMInteractions(trRefEntity_t *ent, trRefLight_t *light)
{
	int                i;
	mdmModel_t         *model      = 0;
	mdmSurfaceIntern_t *mdmSurface = 0;
	shader_t           *shader     = 0;
	byte               cubeSideBits;
	interactionType_t  iaType = IA_DEFAULT;

	/// // don't add third_person objects if not in a portal
	///personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;
	/// // if personalModel is true, nothing is done for real in this function.. better exit now
	///if (personalModel) return;
	if ((ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal)
	{
		return;
	}

	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum and we don't care about proper shadowing
	if (ent->cull == CULL_OUT)
	{
		if (r_shadows->integer <= SHADOWING_BLOB || light->l.noShadows)
		{
			return;
		}
		else
		{
			iaType = IA_SHADOWONLY;
		}
	}

	// avoid drawing of certain objects
#if defined(USE_REFENTITY_NOSHADOWID)
	if (light->l.inverseShadows)
	{
		if (iaType != IA_LIGHTONLY && (light->l.noShadowID && (light->l.noShadowID != ent->e.noShadowID)))
		{
			return;
		}
	}
	else
	{
		if (iaType != IA_LIGHTONLY && (light->l.noShadowID && (light->l.noShadowID == ent->e.noShadowID)))
		{
			return;
		}
	}
#endif

	model = tr.currentModel->mdm;

	// do a quick AABB cull
	if (!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], ent->worldBounds[0], ent->worldBounds[1]))
	{
		tr.pc.c_dlightSurfacesCulled += model->numSurfaces;
		return;
	}

	// do a more expensive and precise light frustum cull
	if (!r_noLightFrustums->integer)
	{
		if (R_CullLightWorldBounds(light, ent->worldBounds) == CULL_OUT)
		{
			tr.pc.c_dlightSurfacesCulled += model->numSurfaces;
			return;
		}
	}

	cubeSideBits = R_CalcLightCubeSideBits(light, ent->worldBounds);

	// generate interactions with all surfaces
	if (r_vboModels->integer && model->numVBOSurfaces && glConfig2.vboVertexSkinningAvailable)
	{
		int             i;
		srfVBOMDMMesh_t *vboSurface;
		shader_t        *shader;

		// static VBOs are fine for lighting and shadow mapping
		for (i = 0; i < model->numVBOSurfaces; i++)
		{
			vboSurface = model->vboSurfaces[i];
			mdmSurface = vboSurface->mdmSurface;

			shader = GetMDMSurfaceShader(ent, mdmSurface);

			// skip all surfaces that don't matter for lighting only pass
			if (shader->isSky || (!shader->interactLight && shader->noShadows))
			{
				continue;
			}

			// we will add shadows even if the main object isn't visible in the view

			R_AddLightInteraction(light, (surfaceType_t *)vboSurface, shader, cubeSideBits, iaType);
			tr.pc.c_dlightSurfaces++;
		}
	}
	else
	{
		for (i = 0, mdmSurface = model->surfaces; i < model->numSurfaces; i++, mdmSurface++)
		{
			shader = GetMDMSurfaceShader(ent, mdmSurface);

			// skip all surfaces that don't matter for lighting only pass
			if (shader->isSky || (!shader->interactLight && shader->noShadows))
			{
				continue;
			}

			// we will add shadows even if the main object isn't visible in the view

			R_AddLightInteraction(light, (surfaceType_t *)mdmSurface, shader, cubeSideBits, iaType);
			tr.pc.c_dlightSurfaces++;
		}
	}
}

/**
 * @brief LocalMatrixTransformVector
 * @param[in] in
 * @param[in] mat
 * @param[out] out
 */
static ID_INLINE void LocalMatrixTransformVector(vec3_t in, vec3_t mat[3], vec3_t out)
{
#ifndef ETL_SSE
	out[0] = in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2];
	out[1] = in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2];
	out[2] = in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2];
#else
	__m128 xmm0, xmm1, xmm2, xmm3, xmm5, xmm6;
	xmm3 = _mm_loadh_pi(_mm_load_ss(&in[0]), (const __m64 *)(&in[1]));
	xmm0 = _mm_loadh_pi(_mm_load_ss(&mat[0][0]), (const __m64 *)(&mat[0][1]));
	xmm1 = _mm_loadh_pi(_mm_load_ss(&mat[1][0]), (const __m64 *)(&mat[1][1]));
	xmm2 = _mm_loadh_pi(_mm_load_ss(&mat[2][0]), (const __m64 *)(&mat[2][1]));
	xmm0 = _mm_mul_ps(xmm0, xmm3);
	//xmm0 = _mm_hadd_ps(xmm0, xmm0);
	//xmm0 = _mm_hadd_ps(xmm0, xmm0);
	xmm5 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm0, xmm5);      //
	xmm5 = _mm_movehl_ps(xmm5, xmm6);   //
	xmm0 = _mm_add_ss(xmm6, xmm5);      //
	_mm_store_ss(&out[0], xmm0);
	xmm1 = _mm_mul_ps(xmm1, xmm3);
	//xmm1 = _mm_hadd_ps(xmm1, xmm1);
	//xmm1 = _mm_hadd_ps(xmm1, xmm1);
	xmm5 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm1, xmm5);      //
	xmm5 = _mm_movehl_ps(xmm5, xmm6);   //
	xmm1 = _mm_add_ss(xmm6, xmm5);      //
	_mm_store_ss(&out[1], xmm1);
	xmm2 = _mm_mul_ps(xmm2, xmm3);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	xmm5 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm2, xmm5);      //
	xmm5 = _mm_movehl_ps(xmm5, xmm6);   //
	xmm2 = _mm_add_ss(xmm6, xmm5);      //
	_mm_store_ss(&out[2], xmm2);
#endif
}

/*
 * @brief LocalMatrixTransformVectorTranslate
 * @param[in] in
 * @param[in] mat
 * @param[in] tr
 * @param[out] out
 *
 * @note Unused
static ID_INLINE void LocalMatrixTransformVectorTranslate(vec3_t in, vec3_t mat[3], vec3_t tr, vec3_t out)
{
    out[0] = in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2] + tr[0];
    out[1] = in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2] + tr[1];
    out[2] = in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2] + tr[2];
}
*/

/*
 * @brief LocalScaledMatrixTransformVector
 * @param[in] in
 * @param[in] s
 * @param[in] mat
 * @param[out] out
 *
 * @note Unused
static ID_INLINE void LocalScaledMatrixTransformVector(vec3_t in, float s, vec3_t mat[3], vec3_t out)
{
    out[0] = (1.0f - s) * in[0] + s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2]);
    out[1] = (1.0f - s) * in[1] + s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2]);
    out[2] = (1.0f - s) * in[2] + s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2]);
}
*/

/*
 * @brief LocalScaledMatrixTransformVectorTranslate
 * @param[in] in
 * @param[in] s
 * @param[in] mat
 * @param[in] tr
 * @param[out] out
 *
 * @note Unused
static ID_INLINE void LocalScaledMatrixTransformVectorTranslate(vec3_t in, float s, vec3_t mat[3], vec3_t tr, vec3_t out)
{
    out[0] = (1.0f - s) * in[0] + s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2] + tr[0]);
    out[1] = (1.0f - s) * in[1] + s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2] + tr[1]);
    out[2] = (1.0f - s) * in[2] + s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2] + tr[2]);
}
*/

/*
 * @brief LocalScaledMatrixTransformVectorFullTranslate
 * @param[in] in
 * @param[in] s
 * @param[in] mat
 * @param[in] tr
 * @param[out] out
 *
 * @note Unused
static ID_INLINE void LocalScaledMatrixTransformVectorFullTranslate(vec3_t in, float s, vec3_t mat[3], vec3_t tr, vec3_t out)
{
    out[0] = (1.0f - s) * in[0] + s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2]) + tr[0];
    out[1] = (1.0f - s) * in[1] + s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2]) + tr[1];
    out[2] = (1.0f - s) * in[2] + s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2]) + tr[2];
}
*/

/*
 * @brief LocalAddScaledMatrixTransformVectorFullTranslate
 * @param[in] in
 * @param[in] s
 * @param[in] mat
 * @param[in] tr
 * @param[out] out
 *
 * @note Unused
static ID_INLINE void LocalAddScaledMatrixTransformVectorFullTranslate(vec3_t in, float s, vec3_t mat[3], vec3_t tr, vec3_t out)
{
    out[0] += s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2]) + tr[0];
    out[1] += s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2]) + tr[1];
    out[2] += s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2]) + tr[2];
}
*/

/**
 * @brief LocalAddScaledMatrixTransformVectorTranslate
 * @param[in] in
 * @param[in] s
 * @param[in] mat
 * @param[in] tr
 * @param[out] out
 */
static ID_INLINE void LocalAddScaledMatrixTransformVectorTranslate(vec3_t in, float s, vec3_t mat[3], vec3_t tr, vec3_t out)
{
#if 0
	out[0] += s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2] + tr[0]);
	out[1] += s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2] + tr[1]);
	out[2] += s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2] + tr[2]);
#else
#ifndef ETL_SSE
	vec3_t v;
	LocalMatrixTransformVector(in, mat, v);
	VectorAdd(v, tr, v);
	VectorScale(v, s, v);
	VectorAdd(out, v, out);
#else
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	xmm5 = _mm_loadh_pi(_mm_load_ss(&out[0]), (const __m64 *)(&out[1]));
	xmm3 = _mm_loadh_pi(_mm_load_ss(&in[0]), (const __m64 *)(&in[1]));
	xmm0 = _mm_loadh_pi(_mm_load_ss(&mat[0][0]), (const __m64 *)(&mat[0][1]));
	xmm1 = _mm_loadh_pi(_mm_load_ss(&mat[1][0]), (const __m64 *)(&mat[1][1]));
	xmm2 = _mm_loadh_pi(_mm_load_ss(&mat[2][0]), (const __m64 *)(&mat[2][1]));
	xmm7 = _mm_loadh_pi(_mm_load_ss(&tr[0]), (const __m64 *)(&tr[1]));  // xmm7 = z y 0 x
	xmm0 = _mm_mul_ps(xmm0, xmm3);
	//xmm0 = _mm_hadd_ps(xmm0, xmm0);
	//xmm0 = _mm_hadd_ps(xmm0, xmm0);
	xmm4 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm0, xmm4);
	xmm4 = _mm_movehl_ps(xmm4, xmm6);
	xmm0 = _mm_add_ss(xmm6, xmm4);
	xmm1 = _mm_mul_ps(xmm1, xmm3);
	//xmm1 = _mm_hadd_ps(xmm1, xmm1);
	//xmm1 = _mm_hadd_ps(xmm1, xmm1);
	xmm4 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm1, xmm4);
	xmm4 = _mm_movehl_ps(xmm4, xmm6);
	xmm1 = _mm_add_ss(xmm6, xmm4);
	xmm2 = _mm_mul_ps(xmm2, xmm3);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm2, xmm4);
	xmm4 = _mm_movehl_ps(xmm4, xmm6);
	xmm2 = _mm_add_ss(xmm6, xmm4);
	xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0b00000000);  // xmm1 = out2 out2 out1 out1
	xmm0 = _mm_shuffle_ps(xmm0, xmm1, 0b11001100);  // xmm0 = out2 out1   0  out0
	xmm0 = _mm_add_ps(xmm0, xmm7);                  // + tr
	xmm0 = _mm_mul_ps(xmm0, _mm_set_ps1(s));        // * s
	xmm0 = _mm_add_ps(xmm0, xmm5);                  // out +=
	_mm_store_ss(&out[0], xmm0);
	_mm_storeh_pi((__m64 *)(&out[1]), xmm0);
#endif
#endif
}

/**
 * @brief LocalAddScaledMatrixTransformVector
 * @param[in] in
 * @param[in] s
 * @param[in] mat
 * @param[out] out
 */
static ID_INLINE void LocalAddScaledMatrixTransformVector(vec3_t in, float s, vec3_t mat[3], vec3_t out)
{
#if 0
	out[0] += s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2]);
	out[1] += s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2]);
	out[2] += s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2]);
#else
#ifndef ETL_SSE
	vec3_t v;
	LocalMatrixTransformVector(in, mat, v);
	VectorScale(v, s, v);
	VectorAdd(out, v, out);
#else
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
	xmm5 = _mm_loadh_pi(_mm_load_ss(&out[0]), (const __m64 *)(&out[1]));
	xmm3 = _mm_loadh_pi(_mm_load_ss(&in[0]), (const __m64 *)(&in[1]));
	xmm0 = _mm_loadh_pi(_mm_load_ss(&mat[0][0]), (const __m64 *)(&mat[0][1]));
	xmm1 = _mm_loadh_pi(_mm_load_ss(&mat[1][0]), (const __m64 *)(&mat[1][1]));
	xmm2 = _mm_loadh_pi(_mm_load_ss(&mat[2][0]), (const __m64 *)(&mat[2][1]));
	xmm0 = _mm_mul_ps(xmm0, xmm3);
	//xmm0 = _mm_hadd_ps(xmm0, xmm0);
	//xmm0 = _mm_hadd_ps(xmm0, xmm0);
	xmm4 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm0, xmm4);
	xmm4 = _mm_movehl_ps(xmm4, xmm6);
	xmm0 = _mm_add_ss(xmm6, xmm4);
	xmm1 = _mm_mul_ps(xmm1, xmm3);
	//xmm1 = _mm_hadd_ps(xmm1, xmm1);
	//xmm1 = _mm_hadd_ps(xmm1, xmm1);
	xmm4 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm1, xmm4);
	xmm4 = _mm_movehl_ps(xmm4, xmm6);
	xmm1 = _mm_add_ss(xmm6, xmm4);
	xmm2 = _mm_mul_ps(xmm2, xmm3);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm2, xmm4);
	xmm4 = _mm_movehl_ps(xmm4, xmm6);
	xmm2 = _mm_add_ss(xmm6, xmm4);
	xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0b00000000);  // xmm1 = out2 out2 out1 out1
	xmm0 = _mm_shuffle_ps(xmm0, xmm1, 0b11001100);  // xmm0 = out2 out1   0  out0
	xmm0 = _mm_mul_ps(xmm0, _mm_set_ps1(s));        // * s
	xmm0 = _mm_add_ps(xmm0, xmm5);                  // out +=
	_mm_store_ss(&out[0], xmm0);
	_mm_storeh_pi((__m64 *)(&out[1]), xmm0);
#endif
#endif
}

// static float LAVangle;
//static float sp, sy, cp, cy, sr, cr;

#ifndef YD_INGLES
static float LAVangle;

/**
 * @brief LocalAngleVector
 * @param[in] angles
 * @param[out] forward
 */
static ID_INLINE void LocalAngleVector(vec3_t angles, vec3_t forward)
{
	float spp, cp, sy, cy;
	LAVangle = DEG2RAD(angles[YAW]);
	SinCos(LAVangle, sy, cy);
	LAVangle = DEG2RAD(angles[PITCH]);
	SinCos(LAVangle, spp, cp);

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -spp;
}
#endif

/*
 * @brief LocalVectorMA
 * @param[in] org
 * @param[in] dist
 * @param[in] vec
 * @param[out] out
 *
 * @note Unused
static ID_INLINE void LocalVectorMA(vec3_t org, float dist, vec3_t vec, vec3_t out)
{
    out[0] = org[0] + dist * vec[0];
    out[1] = org[1] + dist * vec[1];
    out[2] = org[2] + dist * vec[2];
}
*/

#define ANGLES_SHORT_TO_FLOAT(pf, sh)     { *(pf++) = SHORT2ANGLE(*(sh++)); *(pf++) = SHORT2ANGLE(*(sh++)); *(pf++) = SHORT2ANGLE(*(sh++)); }

/**
 * @brief SLerp_Normal
 * @param[in] from
 * @param[in] to
 * @param[in] tt
 * @param[out] out
 */
static ID_INLINE void SLerp_Normal(vec3_t from, vec3_t to, float tt, vec3_t out)
{
#if 0
	float ft = 1.0f - tt;

	out[0] = from[0] * ft + to[0] * tt;
	out[1] = from[1] * ft + to[1] * tt;
	out[2] = from[2] * ft + to[2] * tt;

	// VectorNormalizeOnly( out );
	VectorNormalizeFast(out);
#else
#ifndef ETL_SSE
	vec3_t vft, vtt;
	float  ft = 1.0f - tt;
	VectorScale(from, ft, vft);
	VectorScale(to, tt, vtt);
	VectorAdd(vft, vtt, out);
	// VectorNormalizeOnly( out );
	VectorNormalizeFast(out);
#else
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm6;
	xmm0 = _mm_loadh_pi(_mm_load_ss(&from[0]), (const __m64 *)(&from[1]));
	xmm1 = _mm_loadh_pi(_mm_load_ss(&to[0]), (const __m64 *)(&to[1]));
	xmm0 = _mm_mul_ps(xmm0, _mm_set_ps1(1.0f - tt));
	xmm1 = _mm_mul_ps(xmm1, _mm_set_ps1(tt));
	xmm0 = _mm_add_ps(xmm0, xmm1);
	xmm2 = xmm0;
	xmm2 = _mm_mul_ps(xmm2, xmm2);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	//xmm2 = _mm_hadd_ps(xmm2, xmm2);
	xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
	xmm6 = _mm_add_ps(xmm2, xmm4);      //
	xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
	xmm2 = _mm_add_ss(xmm6, xmm4);      //
	xmm3 = _mm_rsqrt_ss(xmm2);
	xmm3 = _mm_shuffle_ps(xmm3, xmm3, 0);
	xmm0 = _mm_mul_ps(xmm0, xmm3);
	_mm_store_ss(&out[0], xmm0);
	_mm_storeh_pi((__m64 *)(&out[1]), xmm0);
#endif
#endif
}

// All the angles looked up in the SIN_TABLE, are 16b integers.
// These values need to be scaled down to 12b values.
// That's a bit-shift of 4 bits to the right (effectively scaling 65536 down to 4096 = dividing by 16).
// Shifting bits on integers is cheaper than dividing integers. That's why this bit-fiddling and sine look-up is done.
// It should all be faster than using the sin() & cos() functions directly: That's the whole purpose.
// I just wonder, since the actual data of the SIN_TABLE is stored in tr.sinTable[], if accessing data from there
// is as efficient as data that is kept close to the code that uses it (this file).
// "Modern" computers have all kinds of caching going on, so data is available asap. These mechanisms like the data
// to be close to the code that uses it.
//
#define FUNCTABLE_SHIFT   4
#define SIN_TABLE(i)      tr.sinTable[((i) >> FUNCTABLE_SHIFT)&FUNCTABLE_MASK];
#define COS_TABLE(i)      tr.sinTable[(((i) >> FUNCTABLE_SHIFT) + FUNCTABLE_DIV_4)&FUNCTABLE_MASK];

/**
 * @brief LocalIngleVector
 * @param[in] ingles
 * @param[out] forward
 */
static ID_INLINE void LocalIngleVector(int ingles[3], vec3_t forward)
{
	float sp, cp, sy, cy;
	sy         = SIN_TABLE(ingles[YAW]);
	sp         = SIN_TABLE(ingles[PITCH]);
	cy         = COS_TABLE(ingles[YAW]);
	cp         = COS_TABLE(ingles[PITCH]);
	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
}

/**
 * @brief LocalIngleVectorPY (Pitch Yaw)
 * @param[in] ingles pitch, ingles yaw
 * @param[out] forward
 */
static ID_INLINE void LocalIngleVectorPY(const short pitch, const short yaw, vec3_t forward)
{
	float sp, cp, sy, cy;
	sy         = SIN_TABLE(yaw);
	sp         = SIN_TABLE(pitch);
	cy         = COS_TABLE(yaw);
	cp         = COS_TABLE(pitch);
	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
}

/**
 * @brief InglesToAxis
 * @param[in] ingles
 * @param[out] axis
 */
static void InglesToAxis(int ingles[3], vec3_t axis[3])
{
	// get sine/cosines for angles
	float sy   = SIN_TABLE(ingles[YAW]);
	float sp   = SIN_TABLE(ingles[PITCH]);
	float sr   = SIN_TABLE(ingles[ROLL]);
	float cy   = COS_TABLE(ingles[YAW]);
	float cp   = COS_TABLE(ingles[PITCH]);
	float cr   = COS_TABLE(ingles[ROLL]);
	float srsp = sr * sp;
	float crsp = cr * sp;

#ifndef ETL_SSE
	// calculate axis vecs
	axis[0][0] = cp * cy;
	axis[0][1] = cp * sy;
	axis[0][2] = -sp;

	axis[1][0] = srsp * cy + cr * -sy;
	axis[1][1] = srsp * sy + cr * cy;
	axis[1][2] = sr * cp;

	axis[2][0] = crsp * cy + -sr * -sy;
	axis[2][1] = crsp * sy + -sr * cy;
	axis[2][2] = cr * cp;
#else
	_mm_storeu_ps(&axis[0][0], _mm_set_ps(srsp * cy + cr * -sy, -sp, cp * sy, cp * cy));
	_mm_storeu_ps(&axis[1][1], _mm_set_ps(crsp * sy + -sr * cy, crsp * cy + -sr * -sy, sr * cp, srsp * sy + cr * cy));
	axis[2][2] = cr * cp;
#endif
}

/*
===============================================================================
4x4 Matrices
===============================================================================
*/

/*
 * @brief Matrix4Multiply
 * @param[in] a
 * @param[in] b
 * @param[out] dst
 *
 * @note Unused
static ID_INLINE void Matrix4Multiply(const vec4_t a[4], const vec4_t b[4], vec4_t dst[4])
{
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
*/

/**
 * @brief Matrix4MultiplyInto3x3AndTranslation
 * @param[in] a
 * @param[in] b
 * @param[out] dst
 * @param[out] t
 *
 * @note const usage would require an explicit cast, non ANSI C - see unix/const-arg.c
 */
static ID_INLINE void Matrix4MultiplyInto3x3AndTranslation(/*const*/ vec4_t a[4], /*const*/ vec4_t b[4], vec3_t dst[3], vec3_t t)
{
#ifndef ETL_SSE
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
#else
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	xmm4 = _mm_loadu_ps(&b[0][0]);                  // xmm4 =  b3  b2  b1  b0
	xmm5 = _mm_loadu_ps(&b[1][0]);                  // xmm5 =  b7  b6  b5  b4
	xmm6 = _mm_loadu_ps(&b[2][0]);                  // xmm6 = b11 b10  b9  b8
	xmm7 = _mm_loadu_ps(&b[3][0]);                  // xmm7 = b15 b14 b13 b12

	xmm0 = _mm_loadu_ps(&a[0][0]);                  // xmm0 = a3 a2 a1 a0
	xmm3 = _mm_shuffle_ps(xmm0, xmm0, 0b11111111);  // xmm3 = a3 a3 a3 a3
	xmm2 = _mm_shuffle_ps(xmm0, xmm0, 0b10101010);  // xmm2 = a2 a2 a2 a2
	xmm1 = _mm_shuffle_ps(xmm0, xmm0, 0b01010101);  // xmm1 = a1 a1 a1 a1
	xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0b00000000);  // xmm0 = a0 a0 a0 a0
	xmm3 = _mm_mul_ps(xmm3, xmm7);                  // xmm3 = a3*b15 a3*b14 a3*b13 a3*b12
	xmm2 = _mm_mul_ps(xmm2, xmm6);                  // xmm2 = a2*b11 a2*b10  a2*b9  a2*b8
	xmm1 = _mm_mul_ps(xmm1, xmm5);                  // xmm1 =  a1*b7  a1*b6  a1*b5  a1*b4
	xmm0 = _mm_mul_ps(xmm0, xmm4);                  // xmm0 =  a0*b3  a0*b2  a0*b1  a0*b0
	xmm3 = _mm_add_ps(xmm3, xmm2);
	xmm0 = _mm_add_ps(xmm0, xmm1);
	xmm0 = _mm_add_ps(xmm0, xmm3);
	_mm_storeu_ps(&dst[0][0], xmm0);                // NOTE! this will overwrite dst[1][0]    (still legal memory)
	xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0b11111111);  // xmm0 = w w w w
	_mm_store_ss(&t[0], xmm0);

	xmm0 = _mm_loadu_ps(&a[1][0]);
	xmm3 = _mm_shuffle_ps(xmm0, xmm0, 0b11111111);
	xmm2 = _mm_shuffle_ps(xmm0, xmm0, 0b10101010);
	xmm1 = _mm_shuffle_ps(xmm0, xmm0, 0b01010101);
	xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0b00000000);
	xmm3 = _mm_mul_ps(xmm3, xmm7);
	xmm2 = _mm_mul_ps(xmm2, xmm6);
	xmm1 = _mm_mul_ps(xmm1, xmm5);
	xmm0 = _mm_mul_ps(xmm0, xmm4);
	xmm3 = _mm_add_ps(xmm3, xmm2);
	xmm0 = _mm_add_ps(xmm0, xmm1);
	xmm0 = _mm_add_ps(xmm0, xmm3);
	_mm_storeu_ps(&dst[1][0], xmm0);                // NOTE! this will overwrite dst[2][0]    (still legal memory)
	xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0b11111111);  // xmm0 = w w w w
	_mm_store_ss(&t[1], xmm0);

	xmm0 = _mm_loadu_ps(&a[2][0]);
	xmm3 = _mm_shuffle_ps(xmm0, xmm0, 0b11111111);
	xmm2 = _mm_shuffle_ps(xmm0, xmm0, 0b10101010);
	xmm1 = _mm_shuffle_ps(xmm0, xmm0, 0b01010101);
	xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0b00000000);
	xmm3 = _mm_mul_ps(xmm3, xmm7);
	xmm2 = _mm_mul_ps(xmm2, xmm6);
	xmm1 = _mm_mul_ps(xmm1, xmm5);
	xmm0 = _mm_mul_ps(xmm0, xmm4);
	xmm3 = _mm_add_ps(xmm3, xmm2);
	xmm0 = _mm_add_ps(xmm0, xmm1);
	xmm0 = _mm_add_ps(xmm0, xmm3);
	//_mm_storeu_ps(&dst[2][0], xmm0);				// NOTE! this would overwrite into "dst[3][0]" (out of bounds)
	_mm_store_ss(&dst[2][0], xmm0);                 // store x into dst[2][0]
	xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0b10010011);  // xmm0 = z y x w
	_mm_storeh_pi((__m64 *)(&dst[2][1]), xmm0);     // store y & z into dst[2][1] & dst[2][2]
	_mm_store_ss(&t[2], xmm0);                      // store w into t[2]
#endif
}

/*
 * @brief Matrix4Transpose
 * @param[in] matrix
 * @param[out] transpose
 *
 * @note Unused
static ID_INLINE void Matrix4Transpose(const vec4_t matrix[4], vec4_t transpose[4])
{
    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            transpose[i][j] = matrix[j][i];
        }
    }
}
*/

/*
 * @brief Matrix4FromAxis
 * @param[in] axis
 * @param[out] dst
 *
 * @note Unused
static ID_INLINE void Matrix4FromAxis(const vec3_t axis[3], vec4_t dst[4])
{
    int i, j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            dst[i][j] = axis[i][j];
        }
        dst[3][i] = 0;
        dst[i][3] = 0;
    }
    dst[3][3] = 1;
}
*/

/*
 * @brief Matrix4FromScaledAxis
 * @param[in] axis
 * @param[in] scale
 * @param[out] dst
 *
 * @note Unused
static ID_INLINE void Matrix4FromScaledAxis(const vec3_t axis[3], const float scale, vec4_t dst[4])
{
    int i, j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            dst[i][j] = scale * axis[i][j];
            if (i == j)
            {
                dst[i][j] += 1.0f - scale;
            }
        }
        dst[3][i] = 0;
        dst[i][3] = 0;
    }
    dst[3][3] = 1;
}
*/

/*
 * @brief Matrix4FromTranslation
 * @param[in] t
 * @param[out] dst
 *
 * @note Unused
static ID_INLINE void Matrix4FromTranslation(const vec3_t t, vec4_t dst[4])
{
    int i, j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (i == j)
            {
                dst[i][j] = 1;
            }
            else
            {
                dst[i][j] = 0;
            }
        }
        dst[i][3] = t[i];
        dst[3][i] = 0;
    }
    dst[3][3] = 1;
}
*/

/**
 * @brief Can put an axis rotation followed by a translation directly into one matrix
 * @param[in] axis
 * @param[in] t
 * @param[out] dst
 *
 * @note const usage would require an explicit cast, non ANSI C - see unix/const-arg.c
 */
static ID_INLINE void Matrix4FromAxisPlusTranslation(/*const*/ vec3_t axis[3], const vec3_t t, vec4_t dst[4])
{
#ifndef ETL_SSE
	// row 0
	VectorCopy(axis[0], (vec_t *)&dst[0]);
	dst[0][3] = t[0];
	// row 1
	VectorCopy(axis[1], (vec_t *)&dst[1]);
	dst[1][3] = t[1];
	// row 2
	VectorCopy(axis[2], (vec_t *)&dst[2]);
	dst[2][3] = t[2];
	// row 3
	dst[3][0] = dst[3][1] = dst[3][2] = 0.0f;
	dst[3][3] = 1.0f;
#else
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm7;
	// t
	xmm7 = _mm_load_ss(&t[0]);                                  // xmm7 = 0 0 0 tx
	xmm7 = _mm_loadh_pi(xmm7, (const __m64 *)(&t[1]));          // xmm7 = tz ty 0 tx
	// axis
	xmm0 = _mm_loadu_ps(&axis[0][0]);                           // xmm0 = a1x a0z a0y a0x
	xmm3 = _mm_loadu_ps(&axis[1][1]);                           // xmm3 = a2y a2x a1z a1y
	xmm4 = _mm_load_ss(&axis[2][2]);                            // xmm4 = 0 0 0 a2z
	// row 2
	xmm4 = _mm_shuffle_ps(xmm4, xmm7, 0b11010100);              // xmm4 = tz 0 0 a2z
	xmm2 = _mm_shuffle_ps(xmm3, xmm4, 0b11001110);              // xmm2 = tz a2z a2y a2x
	// row 1
	xmm5 = _mm_shuffle_ps(xmm3, xmm7, 0b10100100);              // xmm5 = ty ty a1z a1y
	xmm3 = _mm_shuffle_ps(xmm3, xmm0, 0b11110100);              // xmm3 = a1x a1x a1z a1y
	xmm1 = _mm_shuffle_ps(xmm3, xmm5, 0b11010011);              // xmm1 = ty a1z a1y a1x
	// row 0
	xmm4 = _mm_shuffle_ps(xmm0, xmm7, 0b00001010);              // xmm4 = tx tx a0z a0z
	xmm0 = _mm_shuffle_ps(xmm0, xmm4, 0b11000100);              // xmm0 = tx a0z a0y a0x
	// store rows 0 to 2 (axis | t)
	_mm_storeu_ps((float *)&dst[0], xmm0);
	_mm_storeu_ps((float *)&dst[1], xmm1);
	_mm_storeu_ps((float *)&dst[2], xmm2);
	// the bottom row[3] is filled with: 0 0 0 1
	_mm_storeu_ps((float *)&dst[3], _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)); // high/low order
#endif
}

/**
 * @brief Can put a scaled axis rotation followed by a translation directly into one matrix
 * @param[in] axis
 * @param[in] scale
 * @param[in] t
 * @param[out] dst
 *
 * @note const usage would require an explicit cast, non ANSI C - see unix/const-arg.c
 */
static ID_INLINE void Matrix4FromScaledAxisPlusTranslation(/*const*/ vec3_t axis[3], const float scale, const vec3_t t, vec4_t dst[4])
{
//#ifndef ETL_SSE
/*	int i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			dst[i][j] = scale * axis[i][j];
			if (i == j)
			{
				dst[i][j] += 1.0f - scale;
			}
		}
		dst[3][i] = 0.f;
		dst[i][3] = t[i];
	}
	dst[3][3] = 1.f;*/

	float scale1 = 1.0f - scale;
	VectorScale(axis[0], scale, &dst[0][0]);
	VectorScale(axis[1], scale, &dst[1][0]);
	VectorScale(axis[2], scale, &dst[2][0]);
	dst[0][0] += scale1;
	dst[1][1] += scale1;
	dst[2][2] += scale1;
	dst[0][3]  = t[0];
	dst[1][3]  = t[1];
	dst[2][3]  = t[2];
#ifndef ETL_SSE
	dst[3][0] = dst[3][1] = dst[3][2] = 0.0f;
	dst[3][3] = 1.0f;
#else
	_mm_storeu_ps(&dst[3][0], _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f));
#endif
}

/*
 * @brief Matrix4FromScale
 * @param[in] scale
 * @param[out] dst
 *
 * @note Unused
static ID_INLINE void Matrix4FromScale(const float scale, vec4_t dst[4])
{
    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (i == j)
            {
                dst[i][j] = scale;
            }
            else
            {
                dst[i][j] = 0;
            }
        }
    }
    dst[3][3] = 1;
}
*/

/*
 * @brief Matrix4TransformVector
 * @param[in] m
 * @param[in] src
 * @param[out] dst
 *
 * @note Unused
static ID_INLINE void Matrix4TransformVector(const vec4_t m[4], const vec3_t src, vec3_t dst)
{
    dst[0] = m[0][0] * src[0] + m[0][1] * src[1] + m[0][2] * src[2] + m[0][3];
    dst[1] = m[1][0] * src[0] + m[1][1] * src[1] + m[1][2] * src[2] + m[1][3];
    dst[2] = m[2][0] * src[0] + m[2][1] * src[1] + m[2][2] * src[2] + m[2][3];
}
*/

/*
===============================================================================
3x3 Matrices
===============================================================================
*/

/**
 * @brief Matrix3Transpose
 * @param[in] matrix
 * @param[out] transpose
 */
static ID_INLINE void Matrix3Transpose(const vec3_t matrix[3], vec3_t transpose[3])
{
#ifndef ETL_SSE
	/*
	int i, j;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			transpose[i][j] = matrix[j][i];
		}
	}
	*/
	// unrolled
	transpose[0][0] = matrix[0][0];
	transpose[0][1] = matrix[1][0];
	transpose[0][2] = matrix[2][0];
	transpose[1][0] = matrix[0][1];
	transpose[1][1] = matrix[1][1];
	transpose[1][2] = matrix[2][1];
	transpose[2][0] = matrix[0][2];
	transpose[2][1] = matrix[1][2];
	transpose[2][2] = matrix[2][2];
#else
	__m128 xmm0, xmm1, xmm3, xmm4, xmm5;
	xmm0 = _mm_loadu_ps(&matrix[0][0]);                         // xmm0 = m10 m02 m01 m00
	xmm1 = _mm_loadu_ps(&matrix[1][1]);                         // xmm1 = m21 m20 m12 m11
	xmm3 = _mm_shuffle_ps(xmm0, xmm1, 0b10100100);              // xmm3 = m20 m20 m01 m00
	xmm4 = _mm_shuffle_ps(xmm0, xmm3, 0b01111100);              // xmm4 = m01 m20 m10 m00
	_mm_storeu_ps(&transpose[0][0], xmm4);
	xmm3 = _mm_shuffle_ps(xmm0, xmm1, 0b01011010);              // xmm3 = m12 m12 m02 m02
	xmm5 = _mm_shuffle_ps(xmm1, xmm3, 0b11011100);              // xmm5 = m12 m02 m21 m11
	_mm_storeu_ps(&transpose[1][1], xmm5);
	_mm_store_ss(&transpose[2][2], _mm_load_ss(&matrix[2][2]));
#endif
}

/**
 * @brief R_CalcBone
 * @param[in] torsoParent
 * @param refent - unused
 * @param[in] boneNum
 */
static void R_CalcBone(const int torsoParent, const refEntity_t *refent, int boneNum)
{
#ifndef YD_INGLES
	float *pf;
#endif
	short                    *sh;
	vec3_t                   vec, v2;
	qboolean                 isTorso = qfalse, fullTorso = qfalse;
	mdxBoneFrameCompressed_t *cBonePtr, *cTBonePtr;
	mdxBoneFrame_t           *bonePtr, *parentBone;
	vec3_t                   angles, tangles;

	thisBoneInfo = &boneInfo[boneNum];
	if (thisBoneInfo->torsoWeight != 0.f)
	{
		cTBonePtr = &cBoneListTorso[boneNum];
		isTorso   = qtrue;
		if (thisBoneInfo->torsoWeight == 1.0f)
		{
			fullTorso = qtrue;
		}
	}
	else
	{
		isTorso   = qfalse;
		fullTorso = qfalse;
	}
	cBonePtr = &cBoneList[boneNum];

	bonePtr = &bones[boneNum];

	// we can assume the parent has already been uncompressed for this frame + lerp
	if (thisBoneInfo->parent >= 0)
	{
		parentBone     = &bones[thisBoneInfo->parent];
		parentBoneInfo = &boneInfo[thisBoneInfo->parent];
	}
	else
	{
		parentBone     = NULL;
		parentBoneInfo = NULL;
	}

	// rotation
	if (fullTorso)
	{
		_Short3Vector(cTBonePtr->angles, angles);
	}
	else
	{
		_Short3Vector(cBonePtr->angles, angles);
		if (isTorso)
		{
			int    j;
			vec3_t diff;

			_Short3Vector(cTBonePtr->angles, tangles);
			// blend the angles together
			VectorSubtract(tangles, angles, diff);
			for (j = 0; j < 3; j++)
			{
				if (Q_fabs(diff[j]) > 180.0f)
				{
					diff[j] = AngleNormalize180(diff[j]);
				}
			}
			VectorScale(diff, thisBoneInfo->torsoWeight, diff);
			VectorAdd(angles, diff, angles);
		}
	}
	AnglesToAxis(angles, bonePtr->matrix);

	// translation
	if (parentBone)
	{
		if (fullTorso)
		{
#ifndef YD_INGLES
			sh      = (short *)cTBonePtr->ofsAngles;
			pf      = angles;
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = 0.f;
			LocalAngleVector(angles, vec);
#else
			sh = (short *)cTBonePtr->ofsAngles;
			LocalIngleVectorPY(sh[PITCH], sh[YAW], vec);
#endif

			VectorMA(parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation);
		}
		else
		{
#ifndef YD_INGLES
			sh      = (short *)cBonePtr->ofsAngles;
			pf      = angles;
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = 0.f;
			LocalAngleVector(angles, vec);
#else
			sh = (short *)cBonePtr->ofsAngles;
			LocalIngleVectorPY(sh[PITCH], sh[YAW], vec);
#endif
			if (isTorso)
			{
#ifndef YD_INGLES
				sh      = (short *)cTBonePtr->ofsAngles;
				pf      = tangles;
				*(pf++) = SHORT2ANGLE(*(sh++));
				*(pf++) = SHORT2ANGLE(*(sh++));
				*(pf++) = 0.f;
				LocalAngleVector(tangles, v2);
#else
				sh = (short *)cTBonePtr->ofsAngles;
				LocalIngleVectorPY(sh[PITCH], sh[YAW], v2);
#endif
				// blend the angles together
				SLerp_Normal(vec, v2, thisBoneInfo->torsoWeight, vec);
				VectorMA(parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation);
			}
			else        // legs bone
			{
				VectorMA(parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation);
			}
		}
	}
	else        // just use the frame position
	{
		VectorCopy(frame->parentOffset, bonePtr->translation);
	}

	if (boneNum == torsoParent)     // this is the torsoParent
	{
		VectorCopy(bonePtr->translation, torsoParentOffset);
	}

	validBones[boneNum] = 1;

	rawBones[boneNum] = *bonePtr;
	newBones[boneNum] = 1;
}

/**
 * @brief R_CalcBoneLerp
 * @param[in] torsoParent
 * @param[in] refent
 * @param[in] boneNum
 */
static void R_CalcBoneLerp(const int torsoParent, const refEntity_t *refent, int boneNum, float frontlerp, float backlerp, float torsoFrontlerp, float torsoBacklerp)
{
	int j;
#ifndef YD_INGLES
	float diff, a1, a2;
	float *pf;
#endif
	short                    *sh, *sh2;
	vec3_t                   vec, v2, dir;
	qboolean                 isTorso = qfalse, fullTorso = qfalse;
	mdxBoneFrameCompressed_t *cBonePtr, *cTBonePtr, *cOldBonePtr, *cOldTBonePtr;
	mdxBoneFrame_t           *bonePtr, *parentBone;
	int                      ingles[3], tingles[3];

	if (!refent || boneNum < 0 || boneNum >= MDX_MAX_BONES)
	{
		return;
	}

	thisBoneInfo = &boneInfo[boneNum];

	if (!thisBoneInfo)
	{
		return;
	}

	if (thisBoneInfo->parent >= 0)
	{
		parentBone     = &bones[thisBoneInfo->parent];
		parentBoneInfo = &boneInfo[thisBoneInfo->parent];
	}
	else
	{
		parentBone     = NULL;
		parentBoneInfo = NULL;
	}

	if (thisBoneInfo->torsoWeight != 0.f)
	{
		cTBonePtr    = &cBoneListTorso[boneNum];
		cOldTBonePtr = &cOldBoneListTorso[boneNum];
		isTorso      = qtrue;
		if (thisBoneInfo->torsoWeight == 1.0f)
		{
			fullTorso = qtrue;
		}
	}
	else
	{
		isTorso   = qfalse;
		fullTorso = qfalse;
	}
	cBonePtr    = &cBoneList[boneNum];
	cOldBonePtr = &cOldBoneList[boneNum];

	bonePtr = &bones[boneNum];

	newBones[boneNum] = 1;

	// rotation (take into account 170 to -170 lerps, which need to take the shortest route)
#ifndef YD_INGLES
	if (fullTorso)
	{
		sh  = (short *)cTBonePtr->angles;
		sh2 = (short *)cOldTBonePtr->angles;
		pf  = angles;

		a1      = SHORT2ANGLE(*(sh++));
		a2      = SHORT2ANGLE(*(sh2++));
		diff    = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - torsoBacklerp * diff;
		a1      = SHORT2ANGLE(*(sh++));
		a2      = SHORT2ANGLE(*(sh2++));
		diff    = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - torsoBacklerp * diff;
		a1      = SHORT2ANGLE(*(sh++));
		a2      = SHORT2ANGLE(*(sh2++));
		diff    = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - torsoBacklerp * diff;
	}
	else
	{
		sh  = (short *)cBonePtr->angles;
		sh2 = (short *)cOldBonePtr->angles;
		pf  = angles;

		a1      = SHORT2ANGLE(*(sh++));
		a2      = SHORT2ANGLE(*(sh2++));
		diff    = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - backlerp * diff;
		a1      = SHORT2ANGLE(*(sh++));
		a2      = SHORT2ANGLE(*(sh2++));
		diff    = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - backlerp * diff;
		a1      = SHORT2ANGLE(*(sh++));
		a2      = SHORT2ANGLE(*(sh2++));
		diff    = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - backlerp * diff;

		if (isTorso)
		{
			sh  = (short *)cTBonePtr->angles;
			sh2 = (short *)cOldTBonePtr->angles;
			pf  = tangles;

			a1      = SHORT2ANGLE(*(sh++));
			a2      = SHORT2ANGLE(*(sh2++));
			diff    = AngleNormalize180(a1 - a2);
			*(pf++) = a1 - torsoBacklerp * diff;
			a1      = SHORT2ANGLE(*(sh++));
			a2      = SHORT2ANGLE(*(sh2++));
			diff    = AngleNormalize180(a1 - a2);
			*(pf++) = a1 - torsoBacklerp * diff;
			a1      = SHORT2ANGLE(*(sh++));
			a2      = SHORT2ANGLE(*(sh2++));
			diff    = AngleNormalize180(a1 - a2);
			*(pf++) = a1 - torsoBacklerp * diff;

			// blend the angles together
			for (j = 0; j < 3; j++)
			{
				diff = tangles[j] - angles[j];
				if (Q_fabs(diff) > 180)
				{
					diff = AngleNormalize180(diff);
				}
				angles[j] = angles[j] + thisBoneInfo->torsoWeight * diff;
			}
		}
	}

	AnglesToAxis(angles, bonePtr->matrix);

#else
	// All this old short integer fiddling would probably run faster if regular floats were used.
	// ..especially when SSE is used. (:) something for the upcoming 64b version)

	// ingles-based bone code
	if (fullTorso)
	{
		sh  = (short *)cTBonePtr->angles;
		sh2 = (short *)cOldTBonePtr->angles;
		for (j = 0; j < 3; j++)
		{
			ingles[j] = (sh[j] - sh2[j]) & 65535;
			if (ingles[j] > 32767)
			{
				ingles[j] -= 65536;
			}
			// if you '& 65535', and then check if there is the highest bit (of the result) is set,
			// you could just as well AND with (65535 >> 1)
			ingles[j] = (int)sh[j] - (int)(torsoBacklerp * (float)ingles[j]); // 3 types of data-types in this line..
		}
	}
	else
	{
		sh  = (short *)cBonePtr->angles;
		sh2 = (short *)cOldBonePtr->angles;
		for (j = 0; j < 3; j++)
		{
			ingles[j] = (sh[j] - sh2[j]) & 65535;
			if (ingles[j] > 32767)
			{
				ingles[j] -= 65536;
			}
			ingles[j] = sh[j] - backlerp * ingles[j];
		}

		if (isTorso)
		{
			sh  = (short *)cTBonePtr->angles;
			sh2 = (short *)cOldTBonePtr->angles;
			for (j = 0; j < 3; j++)
			{
				tingles[j] = (sh[j] - sh2[j]) & 65535;
				if (tingles[j] > 32767)
				{
					tingles[j] -= 65536;
				}
				tingles[j] = sh[j] - torsoBacklerp * tingles[j];

				// blend torso and angles
				tingles[j] = (tingles[j] - ingles[j]) & 65535;
				if (tingles[j] > 32767)
				{
					tingles[j] -= 65536;
				}
				ingles[j] += thisBoneInfo->torsoWeight * tingles[j];
			}
		}
	}

	InglesToAxis(ingles, bonePtr->matrix);

#endif

	if (parentBone)
	{
		if (fullTorso)
		{
			sh  = (short *)cTBonePtr->ofsAngles;
			sh2 = (short *)cOldTBonePtr->ofsAngles;
		}
		else
		{
			sh  = (short *)cBonePtr->ofsAngles;
			sh2 = (short *)cOldBonePtr->ofsAngles;
		}

#ifndef YD_INGLES
		pf      = angles;
		*(pf++) = SHORT2ANGLE(*(sh++));
		*(pf++) = SHORT2ANGLE(*(sh++));
		*(pf++) = 0.f;
		LocalAngleVector(angles, v2);   // new

		pf      = angles;
		*(pf++) = SHORT2ANGLE(*(sh2++));
		*(pf++) = SHORT2ANGLE(*(sh2++));
		*(pf++) = 0.f;
		LocalAngleVector(angles, vec);  // old
#else
		LocalIngleVectorPY(sh[PITCH], sh[YAW], v2);

		LocalIngleVectorPY(sh2[PITCH], sh2[YAW], vec);
#endif

		// blend the angles together
		if (fullTorso)
		{
			SLerp_Normal(vec, v2, torsoFrontlerp, dir);
		}
		else
		{
			SLerp_Normal(vec, v2, frontlerp, dir);
		}

		// translation
		if (!fullTorso && isTorso)
		{
			// partial legs/torso, need to lerp according to torsoWeight

			// calc the torso frame
			sh  = (short *)cTBonePtr->ofsAngles;
			sh2 = (short *)cOldTBonePtr->ofsAngles;

#ifndef YD_INGLES
			pf      = angles;
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = 0.f;
			LocalAngleVector(angles, v2);   // new

			pf      = angles;
			*(pf++) = SHORT2ANGLE(*(sh2++));
			*(pf++) = SHORT2ANGLE(*(sh2++));
			*(pf++) = 0.f;
			LocalAngleVector(angles, vec);  // old
#else
			/*ingles[0] = sh[0];
			ingles[1] = sh[1];
			ingles[2] = 0;
			LocalIngleVector(ingles, v2);   // new*/
			LocalIngleVectorPY(sh[PITCH], sh[YAW], v2);
			ingles[2] = 0;
			LocalIngleVectorPY(sh2[PITCH], sh2[YAW], vec);
#endif

			// blend the angles together
			SLerp_Normal(vec, v2, torsoFrontlerp, v2);

			// blend the torso/legs together
			SLerp_Normal(dir, v2, thisBoneInfo->torsoWeight, dir);
		}

		VectorMA(parentBone->translation, thisBoneInfo->parentDist, dir, bonePtr->translation);
	}
	else
	{
		// just interpolate the frame positions
		VectorScale(frame->parentOffset, frontlerp, vec);
		VectorScale(oldFrame->parentOffset, backlerp, v2);
		VectorAdd(vec, v2, bonePtr->translation);
	}

	if (boneNum == torsoParent)
	{
		// this is the torsoParent
		VectorCopy(bonePtr->translation, torsoParentOffset);
	}
	validBones[boneNum] = 1;

	rawBones[boneNum] = *bonePtr;
	newBones[boneNum] = 1;
}

/**
 * @brief R_BonesStillValid
 * @param[in] refent
 * @return
 *
 * @todo FIXME: optimization opportunity here, profile which values change most often and check for those first to get early outs
 *
 * Other way we could do this is doing a random memory probe, which in worst case scenario ends up being the memcmp? - BAD as only a few values are used
 *
 * Another solution: bones cache on an entity basis?
 */
static qboolean R_BonesStillValid(const refEntity_t *refent)
{
#if 1
	if (lastBoneEntity.hModel != refent->hModel)
	{
		return qfalse;
	}
	else if (lastBoneEntity.frame != refent->frame)
	{
		return qfalse;
	}
	else if (lastBoneEntity.oldframe != refent->oldframe)
	{
		return qfalse;
	}
	else if (lastBoneEntity.frameModel != refent->frameModel)
	{
		return qfalse;
	}
	else if (lastBoneEntity.oldframeModel != refent->oldframeModel)
	{
		return qfalse;
	}
	else if (lastBoneEntity.backlerp != refent->backlerp)
	{
		return qfalse;
	}
	else if (lastBoneEntity.torsoFrame != refent->torsoFrame)
	{
		return qfalse;
	}
	else if (lastBoneEntity.oldTorsoFrame != refent->oldTorsoFrame)
	{
		return qfalse;
	}
	else if (lastBoneEntity.torsoFrameModel != refent->torsoFrameModel)
	{
		return qfalse;
	}
	else if (lastBoneEntity.oldTorsoFrameModel != refent->oldTorsoFrameModel)
	{
		return qfalse;
	}
	else if (lastBoneEntity.torsoBacklerp != refent->torsoBacklerp)
	{
		return qfalse;
	}
	else if (lastBoneEntity.reFlags != refent->reFlags)
	{
		return qfalse;
	}
	else if (!VectorCompare(lastBoneEntity.torsoAxis[0], refent->torsoAxis[0]) ||
	         !VectorCompare(lastBoneEntity.torsoAxis[1], refent->torsoAxis[1]) ||
	         !VectorCompare(lastBoneEntity.torsoAxis[2], refent->torsoAxis[2]))
	{
		return qfalse;
	}

	return qtrue;
#else
	return qfalse;
#endif
}

/**
 * @brief The list of bones[] should only be built and modified from within here
 * @param[in] refent
 * @param[in] boneList
 * @param[in] numBones
 */
static void R_CalcBones(const refEntity_t *refent, int *boneList, int numBones)
{
	int            i;
	vec3_t         t;
	vec4_t         m1[4], m2[4];
	int            frameSize;
	int            *boneRefs;
	float          frontlerp, backlerp, torsoFrontlerp, torsoBacklerp;
	float          torsoWeight;
	vec3_t         torsoAxis[3];
	mdxBoneFrame_t *bonePtr;
	mdxHeader_t    *mdxFrameHeader         = R_GetModelByHandle(refent->frameModel)->mdx;
	mdxHeader_t    *mdxOldFrameHeader      = R_GetModelByHandle(refent->oldframeModel)->mdx;
	mdxHeader_t    *mdxTorsoFrameHeader    = R_GetModelByHandle(refent->torsoFrameModel)->mdx;
	mdxHeader_t    *mdxOldTorsoFrameHeader = R_GetModelByHandle(refent->oldTorsoFrameModel)->mdx;

	if (!mdxFrameHeader || !mdxOldFrameHeader || !mdxTorsoFrameHeader || !mdxOldTorsoFrameHeader)
	{
		return;
	}

	// if the entity has changed since the last time the bones were built, reset them

	if (!R_BonesStillValid(refent))
	{
		// different, cached bones are not valid
		Com_Memset(validBones, 0, mdxFrameHeader->numBones);
		lastBoneEntity = *refent;

		// also reset these counter statics
		// print stats for the complete model (not per-surface)
		if (r_showSkeleton->integer == 4 && totalrt)
		{
			Ren_Print("verts %4d/%4d  tris %4d/%4d  (%.2f%%)\n",
			          totalrv, totalv, totalrt, totalt, (float)(100.0 * totalrt) / (float)totalt);
		}
		totalrv = totalrt = totalv = totalt = 0;
	}

	Com_Memset(newBones, 0, mdxFrameHeader->numBones);

	if (refent->oldframe == refent->frame && refent->oldframeModel == refent->frameModel)
	{
		backlerp  = 0.0f;
		frontlerp = 1.0f;
	}
	else
	{
		backlerp  = refent->backlerp;
		frontlerp = 1.0f - backlerp;
	}

	if (refent->oldTorsoFrame == refent->torsoFrame && refent->oldTorsoFrameModel == refent->oldframeModel)
	{
		torsoBacklerp  = 0.0f;
		torsoFrontlerp = 1.0f;
	}
	else
	{
		torsoBacklerp  = refent->torsoBacklerp;
		torsoFrontlerp = 1.0f - torsoBacklerp;
	}

	frame = (mdxFrame_t *) ((byte *) mdxFrameHeader + mdxFrameHeader->ofsFrames +
	                        refent->frame * (int)(sizeof(mdxBoneFrameCompressed_t)) * mdxFrameHeader->numBones +
	                        refent->frame * sizeof(mdxFrame_t));
	torsoFrame = (mdxFrame_t *) ((byte *) mdxTorsoFrameHeader + mdxTorsoFrameHeader->ofsFrames +
	                             refent->torsoFrame * (int)(sizeof(mdxBoneFrameCompressed_t)) * mdxTorsoFrameHeader->numBones +
	                             refent->torsoFrame * sizeof(mdxFrame_t));
	oldFrame = (mdxFrame_t *) ((byte *) mdxOldFrameHeader + mdxOldFrameHeader->ofsFrames +
	                           refent->oldframe * (int)(sizeof(mdxBoneFrameCompressed_t)) * mdxOldFrameHeader->numBones +
	                           refent->oldframe * sizeof(mdxFrame_t));
	oldTorsoFrame = (mdxFrame_t *) ((byte *) mdxOldTorsoFrameHeader + mdxOldTorsoFrameHeader->ofsFrames +
	                                refent->oldTorsoFrame * (int)(sizeof(mdxBoneFrameCompressed_t)) *
	                                mdxOldTorsoFrameHeader->numBones + refent->oldTorsoFrame * sizeof(mdxFrame_t));

	// lerp all the needed bones (torsoParent is always the first bone in the list)

	frameSize = (int)sizeof(mdxBoneFrameCompressed_t) * mdxFrameHeader->numBones;

	cBoneList = (mdxBoneFrameCompressed_t *) ((byte *) mdxFrameHeader + mdxFrameHeader->ofsFrames +
	                                          (refent->frame + 1) * sizeof(mdxFrame_t) + refent->frame * frameSize);
	cBoneListTorso = (mdxBoneFrameCompressed_t *) ((byte *) mdxTorsoFrameHeader + mdxTorsoFrameHeader->ofsFrames +
	                                               (refent->torsoFrame + 1) * sizeof(mdxFrame_t) +
	                                               refent->torsoFrame * frameSize);

	boneInfo = (mdxBoneInfo_t *) ((byte *) mdxFrameHeader + mdxFrameHeader->ofsBones);
	boneRefs = boneList;
	//
	Matrix3Transpose(refent->torsoAxis, torsoAxis);

	if (backlerp == 0.f && torsoBacklerp == 0.f)
	{
		for (i = 0; i < numBones; i++, boneRefs++)
		{
			if (validBones[*boneRefs])
			{
				// this bone is still in the cache
				bones[*boneRefs] = rawBones[*boneRefs];
				continue;
			}

			// find our parent, and make sure it has been calculated
			if ((boneInfo[*boneRefs].parent >= 0) &&
			    (!validBones[boneInfo[*boneRefs].parent] && !newBones[boneInfo[*boneRefs].parent]))
			{
				R_CalcBone(mdxFrameHeader->torsoParent, refent, boneInfo[*boneRefs].parent);
			}

			R_CalcBone(mdxFrameHeader->torsoParent, refent, *boneRefs);
		}
	}
	else
	{
		// interpolated
		cOldBoneList = (mdxBoneFrameCompressed_t *) ((byte *) mdxOldFrameHeader + mdxOldFrameHeader->ofsFrames +
		                                             (refent->oldframe + 1) * sizeof(mdxFrame_t) + refent->oldframe * frameSize);
		cOldBoneListTorso = (mdxBoneFrameCompressed_t *) ((byte *) mdxOldTorsoFrameHeader + mdxOldTorsoFrameHeader->ofsFrames +
		                                                  (refent->oldTorsoFrame + 1) * sizeof(mdxFrame_t) +
		                                                  refent->oldTorsoFrame * frameSize);

		for (i = 0; i < numBones; i++, boneRefs++)
		{
			if (validBones[*boneRefs])
			{
				// this bone is still in the cache
				bones[*boneRefs] = rawBones[*boneRefs];
				continue;
			}

			// find our parent, and make sure it has been calculated
			if ((boneInfo[*boneRefs].parent >= 0) &&
			    (!validBones[boneInfo[*boneRefs].parent] && !newBones[boneInfo[*boneRefs].parent]))
			{
				R_CalcBoneLerp(mdxFrameHeader->torsoParent, refent, boneInfo[*boneRefs].parent, frontlerp, backlerp, torsoFrontlerp, torsoBacklerp);
			}

			R_CalcBoneLerp(mdxFrameHeader->torsoParent, refent, *boneRefs, frontlerp, backlerp, torsoFrontlerp, torsoBacklerp);
		}
	}

	// adjust for torso rotations
	torsoWeight = 0.0f;
	boneRefs    = boneList;
	for (i = 0; i < numBones; i++, boneRefs++)
	{
		thisBoneInfo = &boneInfo[*boneRefs];
		bonePtr      = &bones[*boneRefs];
		// add torso rotation
		if (thisBoneInfo->torsoWeight > 0.0f)
		{
			if (!newBones[*boneRefs])
			{
				// just copy it back from the previous calc
				bones[*boneRefs] = oldBones[*boneRefs];
				continue;
			}

			//if ( !(thisBoneInfo->flags & BONEFLAG_TAG) ) {

			// 1st multiply with the bone->matrix
			// 2nd translation for rotation relative to bone around torso parent offset
			VectorSubtract(bonePtr->translation, torsoParentOffset, t);
			Matrix4FromAxisPlusTranslation(bonePtr->matrix, t, m1);
			// 3rd scaled rotation
			// 4th translate back to torso parent offset
			// use previously created matrix if available for the same weight
			if (torsoWeight != thisBoneInfo->torsoWeight)
			{
				torsoWeight = thisBoneInfo->torsoWeight;
				Matrix4FromScaledAxisPlusTranslation(torsoAxis, torsoWeight, torsoParentOffset, m2);
			}
			// multiply matrices to create one matrix to do all calculations
			Matrix4MultiplyInto3x3AndTranslation(m2, m1, bonePtr->matrix, bonePtr->translation);

			/*
			} else {  // tag's require special handling

			   // rotate each of the axis by the torsoAngles
			   LocalScaledMatrixTransformVector( bonePtr->matrix[0], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[0] );
			   LocalScaledMatrixTransformVector( bonePtr->matrix[1], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[1] );
			   LocalScaledMatrixTransformVector( bonePtr->matrix[2], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[2] );
			   Com_Memcpy( bonePtr->matrix, tmpAxis, sizeof(tmpAxis) );

			   // rotate the translation around the torsoParent
			   VectorSubtract( bonePtr->translation, torsoParentOffset, t );
			   LocalScaledMatrixTransformVector( t, thisBoneInfo->torsoWeight, torsoAxis, bonePtr->translation );
			   VectorAdd( bonePtr->translation, torsoParentOffset, bonePtr->translation );

			}
			*/
		}
	}

	// backup the final bones
	Com_Memcpy(oldBones, bones, sizeof(bones[0]) * mdxFrameHeader->numBones);
}

#ifdef DBG_PROFILE_BONES
#define DBG_SHOWTIME    Ren_Print("%i: %i, ", di++, (dt = ri.Milliseconds()) - ldt); ldt = dt;
#else
#define DBG_SHOWTIME    ;
#endif

/**
 * @brief Tess_MDM_SurfaceAnim
 * @param[in] surface
 */
void Tess_MDM_SurfaceAnim(mdmSurfaceIntern_t *surface)
{
#if 1
	int            i, j, k, i3;
	refEntity_t    *refent;
	int            *boneList;
	mdmModel_t     *mdm;
	md5Vertex_t    *v;
	srfTriangle_t  *tri;
	int            baseIndex, baseVertex;
	float          *tmpPosition, *tmpNormal, *tmpTangent, *tmpBinormal;
	vec3_t         vec;
	float          lodRadius, lodScale;
	int            render_count;
	int32_t        collapse[MDM_MAX_VERTS], *pCollapse, *pCollapseMap;
	mdxBoneFrame_t *bone;

#ifdef DBG_PROFILE_BONES
	int di = 0, dt, ldt;

	dt  = ri.Milliseconds();
	ldt = dt;
#endif

	refent   = &backEnd.currentEntity->e;
	boneList = surface->boneReferences;
	mdm      = surface->model;

	R_CalcBones((const refEntity_t *)refent, boneList, surface->numBoneReferences);

	DBG_SHOWTIME

	// calculate LOD

	// TODO: lerp the radius and origin
	VectorAdd(refent->origin, frame->localOrigin, vec);
	lodRadius = frame->radius;
	lodScale  = RB_CalcMDMLod(refent, vec, lodRadius, mdm->lodBias, mdm->lodScale);

	// debug code
	// lodScale = r_lodTest->value;

//DBG_SHOWTIME

#if 0
	render_count = surface->numVerts; // always render the full surface / all vertices
#else
	// modification to allow dead skeletal bodies to go below minlod (experiment)
	if (refent->reFlags & REFLAG_DEAD_LOD)
	{
		if (lodScale < 0.35f)
		{                       // allow dead to lod down to 35% (even if below surf->minLod) (%35 is arbitrary and probably not good generally.  worked for the blackguard/infantry as a test though)
			lodScale = 0.35f;
		}
		render_count = round((float)surface->numVerts * lodScale);
	}
	else
	{
		render_count = round((float)surface->numVerts * lodScale);
		if (render_count < surface->minLod)
		{
			render_count = surface->minLod;
		}
	}
#endif

	if (render_count > surface->numVerts)
	{
		render_count = surface->numVerts;
	}

	// to profile bone transform performance only
	if (r_showSkeleton->integer == 10)
	{
		return;
	}

	Tess_CheckOverflow(render_count, surface->numTriangles * 3);

	baseIndex  = tess.numIndexes;
	baseVertex = tess.numVertexes;

	// setup triangle list

#if 0
	for (i = 0, tri = surface->triangles; i < surface->numTriangles; i++, tri++)
	{
		tess.indexes[tess.numIndexes + i * 3 + 0] = tess.numVertexes + tri->indexes[0];
		tess.indexes[tess.numIndexes + i * 3 + 1] = tess.numVertexes + tri->indexes[1];
		tess.indexes[tess.numIndexes + i * 3 + 2] = tess.numVertexes + tri->indexes[2];
	}

	tess.numIndexes  += surface->numTriangles * 3;
	tess.numVertexes += render_count;

#else
	// render the full model? (all vertices?)
	if (render_count == surface->numVerts)
	{
		for (i = 0, i3 = tess.numIndexes, tri = surface->triangles; i < surface->numTriangles; i++, i3 += 3, tri++)
		{
			tess.indexes[i3 + 0] = tess.numVertexes + tri->indexes[0];
			tess.indexes[i3 + 1] = tess.numVertexes + tri->indexes[1];
			tess.indexes[i3 + 2] = tess.numVertexes + tri->indexes[2];
		}
		tess.numIndexes  += surface->numTriangles * 3;
		tess.numVertexes += render_count;
	}
	else // render the model with less vertices, using the collapsemap method
	{
		int p0, p1, p2;

		pCollapse = collapse;
		for (j = 0; j < render_count; pCollapse++, j++)
		{
			*pCollapse = j;
		}

		pCollapseMap = &surface->collapseMap[render_count];
		for (j = render_count; j < surface->numVerts; j++, pCollapse++, pCollapseMap++)
		{
			int32_t collapseValue = *pCollapseMap;
			*pCollapse = collapse[collapseValue];
		}

		for (i = 0, tri = surface->triangles; i < surface->numTriangles; i++, tri++)
		{
			p0 = collapse[tri->indexes[0]];
			p1 = collapse[tri->indexes[1]];
			p2 = collapse[tri->indexes[2]];

			// FIXME
			// note:  serious optimization opportunity here,
			//  by sorting the triangles the following "continue"
			//  could have been made into a "break" statement.
			if (p0 == p1 || p1 == p2 || p2 == p0)
			{
				continue;
			}

			tess.indexes[tess.numIndexes + 0] = tess.numVertexes + p0;
			tess.indexes[tess.numIndexes + 1] = tess.numVertexes + p1;
			tess.indexes[tess.numIndexes + 2] = tess.numVertexes + p2;
			tess.numIndexes                  += 3;
		}

		tess.numVertexes += render_count;
	}
#endif

	// deform the vertexes by the lerped bones
	v           = surface->verts;
	tmpPosition = (float *)(&tess.xyz[baseVertex][0]);
	tmpNormal   = (float *)(&tess.normals[baseVertex][0]);
	tmpBinormal = (float *)(&tess.binormals[baseVertex][0]);
	tmpTangent  = (float *)(&tess.tangents[baseVertex][0]);
	for (j = 0; j < render_count; j++, tmpPosition += 4, tmpNormal += 4, tmpTangent += 4, tmpBinormal += 4, v++)
	{
		md5Weight_t *w;

		Vector4Set(tmpPosition, 0.0f, 0.0f, 0.0f, 1.0f);
		Vector4Copy(tmpPosition, tmpTangent);
		Vector4Copy(tmpPosition, tmpBinormal);
		Vector4Copy(tmpPosition, tmpNormal);

		for (k = 0; k < v->numWeights; k++)
		{
			w    = v->weights[k];
			bone = &bones[w->boneIndex];
#ifndef ETL_SSE
			LocalAddScaledMatrixTransformVectorTranslate(w->offset, w->boneWeight, bone->matrix, bone->translation, tmpPosition);
			LocalAddScaledMatrixTransformVector(v->tangent, w->boneWeight, bone->matrix, tmpTangent);
			LocalAddScaledMatrixTransformVector(v->binormal, w->boneWeight, bone->matrix, tmpBinormal);
			LocalAddScaledMatrixTransformVector(v->normal, w->boneWeight, bone->matrix, tmpNormal);
#else
			__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, scale;
			scale = _mm_set_ps1(w->boneWeight);
			xmm0  = _mm_loadh_pi(_mm_load_ss(&bone->matrix[0][0]), (const __m64 *)(&bone->matrix[0][1]));
			xmm1  = _mm_loadh_pi(_mm_load_ss(&bone->matrix[1][0]), (const __m64 *)(&bone->matrix[1][1]));
			xmm2  = _mm_loadh_pi(_mm_load_ss(&bone->matrix[2][0]), (const __m64 *)(&bone->matrix[2][1]));

			xmm5 = _mm_loadh_pi(_mm_load_ss(&tmpPosition[0]), (const __m64 *)(&tmpPosition[1]));
			xmm3 = _mm_loadh_pi(_mm_load_ss(&w->offset[0]), (const __m64 *)(&w->offset[1]));
			xmm7 = _mm_loadh_pi(_mm_load_ss(&bone->translation[0]), (const __m64 *)(&bone->translation[1]));    // xmm7 = z y 0 x
			xmm0 = _mm_mul_ps(xmm0, xmm3);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			xmm4 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm0, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm0 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_mul_ps(xmm1, xmm3);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			xmm4 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm1, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm1 = _mm_add_ss(xmm6, xmm4);      //
			xmm2 = _mm_mul_ps(xmm2, xmm3);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm2, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm2 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0b00000000);  // xmm1 = out2 out2 out1 out1
			xmm0 = _mm_shuffle_ps(xmm0, xmm1, 0b11001100);  // xmm0 = out2 out1   0  out0
			xmm0 = _mm_add_ps(xmm0, xmm7);                  // + tr
			xmm0 = _mm_mul_ps(xmm0, scale);                 // * s
			xmm0 = _mm_add_ps(xmm0, xmm5);                  // out +=
			_mm_store_ss(&tmpPosition[0], xmm0);
			_mm_storeh_pi((__m64 *)(&tmpPosition[1]), xmm0);


			xmm5 = _mm_loadh_pi(_mm_load_ss(&tmpTangent[0]), (const __m64 *)(&tmpTangent[1]));
			xmm3 = _mm_loadh_pi(_mm_load_ss(&v->tangent[0]), (const __m64 *)(&v->tangent[1]));
			xmm0 = _mm_mul_ps(xmm0, xmm3);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			xmm4 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm0, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm0 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_mul_ps(xmm1, xmm3);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			xmm4 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm1, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm1 = _mm_add_ss(xmm6, xmm4);      //
			xmm2 = _mm_mul_ps(xmm2, xmm3);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm2, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm2 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0b00000000);  // xmm1 = out2 out2 out1 out1
			xmm0 = _mm_shuffle_ps(xmm0, xmm1, 0b11001100);  // xmm0 = out2 out1   0  out0
			xmm0 = _mm_mul_ps(xmm0, scale);                 // * s
			xmm0 = _mm_add_ps(xmm0, xmm5);                  // out +=
			_mm_store_ss(&tmpTangent[0], xmm0);
			_mm_storeh_pi((__m64 *)(&tmpTangent[1]), xmm0);


			xmm5 = _mm_loadh_pi(_mm_load_ss(&tmpBinormal[0]), (const __m64 *)(&tmpBinormal[1]));
			xmm3 = _mm_loadh_pi(_mm_load_ss(&v->binormal[0]), (const __m64 *)(&v->binormal[1]));
			xmm0 = _mm_mul_ps(xmm0, xmm3);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			xmm4 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm0, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm0 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_mul_ps(xmm1, xmm3);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			xmm4 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm1, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm1 = _mm_add_ss(xmm6, xmm4);      //
			xmm2 = _mm_mul_ps(xmm2, xmm3);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm2, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm2 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0b00000000);  // xmm1 = out2 out2 out1 out1
			xmm0 = _mm_shuffle_ps(xmm0, xmm1, 0b11001100);  // xmm0 = out2 out1   0  out0
			xmm0 = _mm_mul_ps(xmm0, scale);                 // * s
			xmm0 = _mm_add_ps(xmm0, xmm5);                  // out +=
			_mm_store_ss(&tmpBinormal[0], xmm0);
			_mm_storeh_pi((__m64 *)(&tmpBinormal[1]), xmm0);


			xmm5 = _mm_loadh_pi(_mm_load_ss(&tmpNormal[0]), (const __m64 *)(&tmpNormal[1]));
			xmm3 = _mm_loadh_pi(_mm_load_ss(&v->normal[0]), (const __m64 *)(&v->normal[1]));
			xmm0 = _mm_mul_ps(xmm0, xmm3);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			//xmm0 = _mm_hadd_ps(xmm0, xmm0);
			xmm4 = _mm_movehdup_ps(xmm0);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm0, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm0 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_mul_ps(xmm1, xmm3);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			//xmm1 = _mm_hadd_ps(xmm1, xmm1);
			xmm4 = _mm_movehdup_ps(xmm1);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm1, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm1 = _mm_add_ss(xmm6, xmm4);      //
			xmm2 = _mm_mul_ps(xmm2, xmm3);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			//xmm2 = _mm_hadd_ps(xmm2, xmm2);
			xmm4 = _mm_movehdup_ps(xmm2);       // faster version of: 2 * hadd
			xmm6 = _mm_add_ps(xmm2, xmm4);      //
			xmm4 = _mm_movehl_ps(xmm4, xmm6);   //
			xmm2 = _mm_add_ss(xmm6, xmm4);      //
			xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0b00000000);  // xmm1 = out2 out2 out1 out1
			xmm0 = _mm_shuffle_ps(xmm0, xmm1, 0b11001100);  // xmm0 = out2 out1   0  out0
			xmm0 = _mm_mul_ps(xmm0, scale);                 // * s
			xmm0 = _mm_add_ps(xmm0, xmm5);                  // out +=
			_mm_store_ss(&tmpNormal[0], xmm0);
			_mm_storeh_pi((__m64 *)(&tmpNormal[1]), xmm0);
#endif
		}

		//LocalMatrixTransformVector(v->normal, bones[v->weights[0]->boneIndex].matrix, tempNormal);

		Vector2Copy(v->texCoords, tess.texCoords[baseVertex + j]);
	}

	DBG_SHOWTIME

#if 0
	if (r_showSkeleton->integer)
	{
		GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
		if (r_showSkeleton->integer < 3 || r_showSkeleton->integer == 5 || r_showSkeleton->integer == 8 || r_showSkeleton->integer == 9)
		{
			// DEBUG: show the bones as a stick figure with axis at each bone
			boneRefs = (int *)((byte *) surface + surface->ofsBoneReferences);
			for (i = 0; i < surface->numBoneReferences; i++, boneRefs++)
			{
				bonePtr = &bones[*boneRefs];

				GL_Bind(tr.whiteImage);
				if (r_showSkeleton->integer != 9)
				{
					glLineWidth(1);
					glBegin(GL_LINES);
					for (j = 0; j < 3; j++)
					{
						VectorClear(vec);
						vec[j] = 1;
						glColor3fv(vec);
						glVertex3fv(bonePtr->translation);
						VectorMA(bonePtr->translation, (r_showSkeleton->integer == 8 ? 1.5 : 5), bonePtr->matrix[j], vec);
						glVertex3fv(vec);
					}
					glEnd();
				}

				// connect to our parent if it's valid
				if (validBones[boneInfo[*boneRefs].parent])
				{
					glLineWidth(r_showSkeleton->integer == 8 ? 4 : 2);
					glBegin(GL_LINES);
					glColor3f(.6, .6, .6);
					glVertex3fv(bonePtr->translation);
					glVertex3fv(bones[boneInfo[*boneRefs].parent].translation);
					glEnd();
				}

				glLineWidth(1);
			}

			if (r_showSkeleton->integer == 8)
			{
				// FIXME: Actually draw the whole skeleton
				//if( surface == (mdmSurface_t *)((byte *)header + header->ofsSurfaces) ) {
				mdxHeader_t *mdxHeader = R_GetModelByHandle(refent->frameModel)->mdx;

				boneRefs = (int *)((byte *) surface + surface->ofsBoneReferences);

				glDepthRange(0, 0); // never occluded
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				for (i = 0; i < surface->numBoneReferences; i++, boneRefs++)
				{
					vec3_t        diff;
					mdxBoneInfo_t *mdxBoneInfo =
						(mdxBoneInfo_t *) ((byte *) mdxHeader + mdxHeader->ofsBones + *boneRefs * sizeof(mdxBoneInfo_t));
					bonePtr = &bones[*boneRefs];

					VectorSet(vec, 0.f, 0.f, 32.f);
					VectorSubtract(bonePtr->translation, vec, diff);
					vec[0] = vec[0] + diff[0] * 6;
					vec[1] = vec[1] + diff[1] * 6;
					vec[2] = vec[2] + diff[2] * 3;

					glEnable(GL_BLEND);
					glBegin(GL_LINES);
					glColor4f(1.f, .4f, .05f, .35f);
					glVertex3fv(bonePtr->translation);
					glVertex3fv(vec);
					glEnd();
					glDisable(GL_BLEND);

					R_DebugText(vec, 1.f, 1.f, 1.f, mdxBoneInfo->name, qfalse); // qfalse, as there is no reason to set depthrange again
				}

				glDepthRange(0, 1);
				//}
			}
			else if (r_showSkeleton->integer == 9)
			{
				if (surface == (mdmSurface_t *) ((byte *) header + header->ofsSurfaces))
				{
					mdmTag_t *pTag = (mdmTag_t *) ((byte *) header + header->ofsTags);

					glDepthRange(0, 0); // never occluded
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					for (i = 0; i < header->numTags; i++)
					{
						mdxBoneFrame_t *tagBone;
						orientation_t  outTag;
						vec3_t         diff;

						// now extract the orientation for the bone that represents our tag
						tagBone = &bones[pTag->boneIndex];
						VectorClear(outTag.origin);
						LocalAddScaledMatrixTransformVectorTranslate(pTag->offset, 1.f, tagBone->matrix, tagBone->translation,
						                                             outTag.origin);
						for (j = 0; j < 3; j++)
						{
							LocalMatrixTransformVector(pTag->axis[j], bone->matrix, outTag.axis[j]);
							//VectorTransformM4(bone->matrix, pTag->axis[j], outTag.axis[j]);
						}

						GL_Bind(tr.whiteImage);
						glLineWidth(2);
						glBegin(GL_LINES);
						for (j = 0; j < 3; j++)
						{
							VectorClear(vec);
							vec[j] = 1;
							glColor3fv(vec);
							glVertex3fv(outTag.origin);
							VectorMA(outTag.origin, 5, outTag.axis[j], vec);
							glVertex3fv(vec);
						}
						glEnd();

						VectorSet(vec, 0.f, 0.f, 32.f);
						VectorSubtract(outTag.origin, vec, diff);
						vec[0] = vec[0] + diff[0] * 2;
						vec[1] = vec[1] + diff[1] * 2;
						vec[2] = vec[2] + diff[2] * 1.5;

						glLineWidth(1);
						glEnable(GL_BLEND);
						glBegin(GL_LINES);
						glColor4f(1.f, .4f, .05f, .35f);
						glVertex3fv(outTag.origin);
						glVertex3fv(vec);
						glEnd();
						glDisable(GL_BLEND);

						R_DebugText(vec, 1.f, 1.f, 1.f, pTag->name, qfalse);    // qfalse, as there is no reason to set depthrange again

						pTag = (mdmTag_t *) ((byte *) pTag + pTag->ofsEnd);
					}
					glDepthRange(0, 1);
				}
			}
		}

		if (r_showSkeleton->integer >= 3 && r_showSkeleton->integer <= 6)
		{
			int render_indexes = (tess.numIndexes - oldIndexes);

			// show mesh edges
			tempVert   = (float *)(tess.xyz + baseVertex);
			tempNormal = (float *)(tess.normal + baseVertex);

			GL_Bind(tr.whiteImage);
			glLineWidth(1);
			glBegin(GL_LINES);
			glColor3f(.0, .0, .8);

			pIndexes = &tess.indexes[oldIndexes];
			for (j = 0; j < render_indexes / 3; j++, pIndexes += 3)
			{
				glVertex3fv(tempVert + 4 * pIndexes[0]);
				glVertex3fv(tempVert + 4 * pIndexes[1]);

				glVertex3fv(tempVert + 4 * pIndexes[1]);
				glVertex3fv(tempVert + 4 * pIndexes[2]);

				glVertex3fv(tempVert + 4 * pIndexes[2]);
				glVertex3fv(tempVert + 4 * pIndexes[0]);
			}

			glEnd();

			// track debug stats
			if (r_showSkeleton->integer == 4)
			{
				totalrv += render_count;
				totalrt += render_indexes / 3;
				totalv  += surface->numVerts;
				totalt  += surface->numTriangles;
			}

			if (r_showSkeleton->integer == 3)
			{
				Ren_Print("Lod %.2f  verts %4d/%4d  tris %4d/%4d  (%.2f%%)\n", lodScale, render_count,
				          surface->numVerts, render_indexes / 3, surface->numTriangles,
				          (float)(100.0 * render_indexes / 3) / (float)surface->numTriangles);
			}
		}

		if (r_showSkeleton->integer == 6 || r_showSkeleton->integer == 7)
		{
			v        = (mdmVertex_t *) ((byte *) surface + surface->ofsVerts);
			tempVert = (float *)(tess.xyz + baseVertex);
			GL_Bind(tr.whiteImage);
			glPointSize(5);
			glBegin(GL_POINTS);
			for (j = 0; j < render_count; j++, tempVert += 4)
			{
				if (v->numWeights > 1)
				{
					if (v->numWeights == 2)
					{
						glColor3f(.4f, .4f, 0.f);
					}
					else if (v->numWeights == 3)
					{
						glColor3f(.8f, .4f, 0.f);
					}
					else
					{
						glColor3f(1.f, .4f, 0.f);
					}
					glVertex3fv(tempVert);
				}
				v = (mdmVertex_t *) &v->weights[v->numWeights];
			}
			glEnd();
		}
	}
#endif

/*	if( r_showmodelbounds->integer ) {
        vec3_t diff, v1, v2, v3, v4, v5, v6;
        mdxHeader_t	*mdxHeader = R_GetModelByHandle( refent->frameModel )->mdx;
        mdxFrame_t	*mdxFrame = (mdxFrame_t *)((byte *)mdxHeader + mdxHeader->ofsFrames +
                                refent->frame * (int) ( sizeof( mdxBoneFrameCompressed_t ) ) * mdxHeader->numBones +
                                refent->frame * sizeof(mdxFrame_t));

        // show model bounds
        GL_Bind( tr.whiteImage );
        GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );
        glLineWidth( 1 );
        glColor3f( .0,.8,.0 );
        glBegin( GL_LINES );

        VectorSubtract( mdxFrame->bounds[0], mdxFrame->bounds[1], diff);

        VectorCopy( mdxFrame->bounds[0], v1 );
        VectorCopy( mdxFrame->bounds[0], v2 );
        VectorCopy( mdxFrame->bounds[0], v3 );
        v1[0] -= diff[0];
        v2[1] -= diff[1];
        v3[2] -= diff[2];
        glVertex3fv( mdxFrame->bounds[0] );
        glVertex3fv( v1 );
        glVertex3fv( mdxFrame->bounds[0] );
        glVertex3fv( v2 );
        glVertex3fv( mdxFrame->bounds[0] );
        glVertex3fv( v3 );

        VectorCopy( mdxFrame->bounds[1], v4 );
        VectorCopy( mdxFrame->bounds[1], v5 );
        VectorCopy( mdxFrame->bounds[1], v6 );
        v4[0] += diff[0];
        v5[1] += diff[1];
        v6[2] += diff[2];
        glVertex3fv( mdxFrame->bounds[1] );
        glVertex3fv( v4 );
        glVertex3fv( mdxFrame->bounds[1] );
        glVertex3fv( v5 );
        glVertex3fv( mdxFrame->bounds[1] );
        glVertex3fv( v6 );

        glVertex3fv( v2 );
        glVertex3fv( v6 );
        glVertex3fv( v6 );
        glVertex3fv( v1 );
        glVertex3fv( v1 );
        glVertex3fv( v5 );

        glVertex3fv( v2 );
        glVertex3fv( v4 );
        glVertex3fv( v4 );
        glVertex3fv( v3 );
        glVertex3fv( v3 );
        glVertex3fv( v5 );

        glEnd();
    }*/

	if (r_showSkeleton->integer > 1)
	{
		// dont draw the actual surface
		tess.numIndexes  = baseIndex;
		tess.numVertexes = baseVertex;
		return;
	}

#ifdef DBG_PROFILE_BONES
	Ren_Print("\n");
#endif

#endif // entire function block
}

/**
 * @brief Tess_SurfaceVBOMDMMesh
 * @param[in] surface
 */
void Tess_SurfaceVBOMDMMesh(srfVBOMDMMesh_t *surface)
{
	int                i;
	mdmModel_t         *mdmModel;
	mdmSurfaceIntern_t *mdmSurface;
	refEntity_t        *refent;
	int                lodIndex;
	IBO_t              *lodIBO;
	vec3_t             vec;

	Ren_LogComment("--- Tess_SurfaceVBOMDMMesh ---\n");

	if (!surface->vbo || !surface->ibo[0])
	{
		return;
	}

	Tess_EndBegin();

	R_BindVBO(surface->vbo);

	tess.numVertexes = surface->numVerts;

	mdmModel   = surface->mdmModel;
	mdmSurface = surface->mdmSurface;

	refent = &backEnd.currentEntity->e;

	// RB: R_CalcBones requires the bone references from the original mdmSurface_t because
	// the GPU vertex skinning only requires a subset which does not reference the parent bones of the vertex weights.
	R_CalcBones((const refEntity_t *)refent, mdmSurface->boneReferences, mdmSurface->numBoneReferences);

	tess.vboVertexSkinning = qtrue;

	for (i = 0; i < surface->numBoneRemap; i++)
	{
#if 0
		// there's a bug inside this block..
		MatrixFromVectorsFLU(m, bones[surface->boneRemapInverse[i]].matrix[0],
		                     bones[surface->boneRemapInverse[i]].matrix[1],
		                     bones[surface->boneRemapInverse[i]].matrix[2]);

		Matrix4Transpose(m, m2);

		MatrixSetupTransformFromRotation(tess.boneMatrices[i], m2, bones[surface->boneRemapInverse[i]].translation);
#else
		float *row0  = bones[surface->boneRemapInverse[i]].matrix[0];
		float *row1  = bones[surface->boneRemapInverse[i]].matrix[1];
		float *row2  = bones[surface->boneRemapInverse[i]].matrix[2];
		float *trans = bones[surface->boneRemapInverse[i]].translation;

		float *m = tess.boneMatrices[i];

		Vector4Set(&m[0], row0[0], row1[0], row2[0], 0.f);
		Vector4Set(&m[4], row0[1], row1[1], row2[1], 0.f);
		Vector4Set(&m[8], row0[2], row1[2], row2[2], 0.f);
		Vector4Set(&m[12], trans[0], trans[1], trans[2], 1.f);
#endif
	}

	// calculate LOD

	// TODO: lerp the radius and origin
	VectorAdd(refent->origin, frame->localOrigin, vec);
	lodIndex = RB_CalcMDMLodIndex(refent, vec, frame->radius, mdmModel->lodBias, mdmModel->lodScale, mdmSurface);
	lodIBO   = surface->ibo[lodIndex];

	if (lodIBO == NULL)
	{
		lodIBO = surface->ibo[0];
	}
	//Ren_Print("LOD index = '%i', IBO = '%s'\n", lodIndex, lodIBO->name);

	R_BindIBO(lodIBO);

	tess.numIndexes = lodIBO->indexesNum;

	//GL_VertexAttribPointers(ATTR_BITS | ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS);

	Tess_End();
}

/**
 * @brief R_MDM_GetBoneTag
 * @param[out] outTag
 * @param[in] mdm
 * @param[in] startTagIndex
 * @param[in] refent
 * @param[in] tagName
 * @return
 */
int R_MDM_GetBoneTag(orientation_t *outTag, mdmModel_t *mdm, int startTagIndex, const refEntity_t *refent,
                     const char *tagName)
{
	int            i;
	mdmTagIntern_t *pTag;
	int            *boneList;
	mdxBoneFrame_t *bone;

	if (startTagIndex > mdm->numTags)
	{
		Com_Memset(outTag, 0, sizeof(*outTag));
		return -1;
	}

	// find the correct tag
	pTag = (mdmTagIntern_t *) &mdm->tags[startTagIndex];

#if 1 //ndef ETL_SSE
	for (i = startTagIndex; i < mdm->numTags; i++, pTag++)
	{
		if (!strcmp(pTag->name, tagName))
		{
			break;
		}
	}
#else
	// TODO: make it work..
	// We presume that tagnames are conform the Q3 rules (char[64])
	// but it appears the given 'tagName' is not pointing to a char[64] for real.
	// Probably, if trap_R_LerpTag() is passed a tagName which is stored in a char[64], it works.
	// UPDATE: tested that^^ trap_R_LerpTag(), passing a char[64], and it is indeed working (very well).
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm7, zeroes;
	int     mask0, mask16, mask32;
	zeroes = _mm_setzero_si128();
	xmm0   = _mm_loadu_si128((const __m128i *)tagName[0]);
	mask0  = _mm_movemask_epi8(_mm_cmpeq_epi8(xmm0, zeroes));
	if (mask0 == 0)
	{
		xmm1   = _mm_loadu_si128((const __m128i *)tagName[16]);
		mask16 = _mm_movemask_epi8(_mm_cmpeq_epi8(xmm1, zeroes));
		if (mask16 == 0)
		{
			xmm2   = _mm_loadu_si128((const __m128i *)tagName[32]);
			mask32 = _mm_movemask_epi8(_mm_cmpeq_epi8(xmm2, zeroes));
			if (mask32 == 0)
			{
				xmm3 = _mm_loadu_si128((const __m128i *)tagName[48]);
			}
		}
	}
	for (i = startTagIndex; i < mdm->numTags; i++, pTag++)
	{
		xmm4 = _mm_loadu_si128((const __m128i *)&pTag->name[0]);
		xmm7 = _mm_cmpeq_epi8(xmm4, xmm0);
		if (_mm_movemask_epi8(xmm7) != 0xFFFF)
		{
			continue;
		}
		if (mask0 != 0)
		{
			break;
		}

		xmm4 = _mm_loadu_si128((const __m128i *)&pTag->name[16]);
		xmm7 = _mm_cmpeq_epi8(xmm4, xmm1);
		if (_mm_movemask_epi8(xmm7) != 0xFFFF)
		{
			continue;
		}
		if (mask16 != 0)
		{
			break;
		}

		xmm4 = _mm_loadu_si128((const __m128i *)&pTag->name[32]);
		xmm7 = _mm_cmpeq_epi8(xmm4, xmm2);
		if (_mm_movemask_epi8(xmm7) != 0xFFFF)
		{
			continue;
		}
		if (mask32 != 0)
		{
			break;
		}

		xmm4 = _mm_loadu_si128((const __m128i *)&pTag->name[48]);
		xmm7 = _mm_cmpeq_epi8(xmm4, xmm3);
		if (_mm_movemask_epi8(xmm7) == 0xFFFF)
		{
			break;
		}
	}
#endif

	if (i >= mdm->numTags)
	{
		Com_Memset(outTag, 0, sizeof(*outTag));
		return -1;
	}

	// calc the bones
	boneList = pTag->boneReferences;
	R_CalcBones(refent, boneList, pTag->numBoneReferences);

	// now extract the orientation for the bone that represents our tag
	bone = &bones[pTag->boneIndex];
	VectorClear(outTag->origin);
	LocalAddScaledMatrixTransformVectorTranslate(pTag->offset, 1.f, bone->matrix, bone->translation, outTag->origin);
	// xyz
	LocalMatrixTransformVector(pTag->axis[0], bone->matrix, outTag->axis[0]);
	LocalMatrixTransformVector(pTag->axis[1], bone->matrix, outTag->axis[1]);
	LocalMatrixTransformVector(pTag->axis[2], bone->matrix, outTag->axis[2]);

	return i;
}

#pragma warning(default:4700)
