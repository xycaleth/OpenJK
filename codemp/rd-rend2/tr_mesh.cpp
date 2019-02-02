/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_mesh.c: triangle model functions

#include "tr_local.h"

#include "tr_steroids_render.h"

float ProjectRadius( float r, vec3_t location )
{
	float pr;
	float dist;
	float c;
	vec3_t	p;
	float	projected[4];

	c = DotProduct( tr.viewParms.ori.axis[0], tr.viewParms.ori.origin );
	dist = DotProduct( tr.viewParms.ori.axis[0], location ) - c;

	if ( dist <= 0 )
		return 0;

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

	if ( pr > 1.0f )
		pr = 1.0f;

	return pr;
}

/*
=============
R_CullModel
=============
*/
static int R_CullModel( mdvModel_t *model, trRefEntity_t *ent ) {
	vec3_t		bounds[2];
	mdvFrame_t	*oldFrame, *newFrame;
	int			i;

	return CULL_IN;

	// compute frame pointers
	newFrame = model->frames + ent->e.frame;
	oldFrame = model->frames + ent->e.oldframe;

	// cull bounding sphere ONLY if this is not an upscaled entity
	if ( !ent->e.nonNormalizedAxes )
	{
		if ( ent->e.frame == ent->e.oldframe )
		{
			switch ( R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius ) )
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
		}
		else
		{
			int sphereCull, sphereCullB;

			sphereCull  = R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius );
			if ( newFrame == oldFrame ) {
				sphereCullB = sphereCull;
			} else {
				sphereCullB = R_CullLocalPointAndRadius( oldFrame->localOrigin, oldFrame->radius );
			}

			if ( sphereCull == sphereCullB )
			{
				if ( sphereCull == CULL_OUT )
				{
					tr.pc.c_sphere_cull_md3_out++;
					return CULL_OUT;
				}
				else if ( sphereCull == CULL_IN )
				{
					tr.pc.c_sphere_cull_md3_in++;
					return CULL_IN;
				}
				else
				{
					tr.pc.c_sphere_cull_md3_clip++;
				}
			}
		}
	}
	
	// calculate a bounding box in the current coordinate system
	for (i = 0 ; i < 3 ; i++) {
		bounds[0][i] = oldFrame->bounds[0][i] < newFrame->bounds[0][i] ? oldFrame->bounds[0][i] : newFrame->bounds[0][i];
		bounds[1][i] = oldFrame->bounds[1][i] > newFrame->bounds[1][i] ? oldFrame->bounds[1][i] : newFrame->bounds[1][i];
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
R_ComputeLOD

=================
*/
int R_ComputeLOD( trRefEntity_t *ent, const model_t *model ) {
	float radius;
	float flod, lodscale;
	float projectedRadius;
	mdvFrame_t *frame;
	mdrHeader_t *mdr;
	mdrFrame_t *mdrframe;
	int lod;

	if ( model->numLods < 2 )
	{
		// model has only 1 LOD level, skip computations and bias
		lod = 0;
	}
	else
	{
		// multiple LODs exist, so compute projected bounding sphere
		// and use that as a criteria for selecting LOD

		if(model->type == MOD_MDR)
		{
			int frameSize;
			mdr = model->data.mdr;
			frameSize = (size_t) (&((mdrFrame_t *)0)->bones[mdr->numBones]);
			
			mdrframe = (mdrFrame_t *) ((byte *) mdr + mdr->ofsFrames + frameSize * ent->e.frame);
			
			radius = RadiusFromBounds(mdrframe->bounds[0], mdrframe->bounds[1]);
		}
		else
		{
			//frame = ( md3Frame_t * ) ( ( ( unsigned char * ) tr.currentModel->md3[0] ) + tr.currentModel->md3[0]->ofsFrames );
			frame = model->data.mdv[0]->frames;

			frame += ent->e.frame;

			radius = RadiusFromBounds( frame->bounds[0], frame->bounds[1] );
		}

		if ( ( projectedRadius = ProjectRadius( radius, ent->e.origin ) ) != 0 )
		{
			lodscale = (r_lodscale->value+r_autolodscalevalue->integer);
			if (lodscale > 20) lodscale = 20;
			flod = 1.0f - projectedRadius * lodscale;
		}
		else
		{
			// object intersects near view plane, e.g. view weapon
			flod = 0;
		}

		flod *= model->numLods;
		lod = Q_ftol(flod);

		if ( lod < 0 )
		{
			lod = 0;
		}
		else if ( lod >= model->numLods )
		{
			lod = model->numLods - 1;
		}
	}

	lod += r_lodbias->integer;
	
	if ( lod >= model->numLods )
		lod = model->numLods - 1;
	if ( lod < 0 )
		lod = 0;

	return lod;
}

/*
=================
R_ComputeFogNum

=================
*/
int R_ComputeFogNum( mdvModel_t *model, trRefEntity_t *ent ) {
	int				i, j;
	fog_t			*fog;
	mdvFrame_t		*mdvFrame;
	vec3_t			localOrigin;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}

	// FIXME: non-normalized axis issues
	mdvFrame = model->frames + ent->e.frame;
	VectorAdd( ent->e.origin, mdvFrame->localOrigin, localOrigin );
	for ( i = 1 ; i < tr.world->numfogs ; i++ ) {
		fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( localOrigin[j] - mdvFrame->radius >= fog->bounds[1][j] ) {
				break;
			}
			if ( localOrigin[j] + mdvFrame->radius <= fog->bounds[0][j] ) {
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
=================
R_AddMD3Surfaces

=================
*/
void R_AddMD3Surfaces(
	trRefEntity_t *ent,
	int entityNum,
	const r2::camera_t *camera,
	std::vector<r2::culled_surface_t> &culledSurfaces)
{
	int				i;
	mdvModel_t		*model = nullptr;
	mdvSurface_t	*surface = nullptr;
	shader_t		*shader = nullptr;
	int				cull;
	int				lod;
	int				fogNum;
	int             cubemapIndex;
	qboolean		personalModel;

	// don't add third_person objects if not in a portal
	personalModel = (qboolean)((ent->e.renderfx & RF_THIRD_PERSON) && !(tr.viewParms.isPortal 
	                 || (tr.viewParms.flags & (VPF_SHADOWMAP | VPF_DEPTHSHADOW))));

	const model_t *currentModel = R_GetModelByHandle(ent->e.hModel);

	if ( ent->e.renderfx & RF_WRAP_FRAMES ) {
		ent->e.frame %= currentModel->data.mdv[0]->numFrames;
		ent->e.oldframe %= currentModel->data.mdv[0]->numFrames;
	}

	//
	// Validate the frames so there is no chance of a crash.
	// This will write directly into the entity structure, so
	// when the surfaces are rendered, they don't need to be
	// range checked again.
	//
	if ( (ent->e.frame >= currentModel->data.mdv[0]->numFrames)
		|| (ent->e.frame < 0)
		|| (ent->e.oldframe >= currentModel->data.mdv[0]->numFrames)
		|| (ent->e.oldframe < 0) ) {
			ri.Printf( PRINT_DEVELOPER, "R_AddMD3Surfaces: no such frame %d to %d for '%s'\n",
				ent->e.oldframe, ent->e.frame,
				currentModel->name );
			ent->e.frame = 0;
			ent->e.oldframe = 0;
	}

	//
	// compute LOD
	//
	lod = R_ComputeLOD( ent, currentModel );

	model = currentModel->data.mdv[lod];

	//
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum.
	//
	cull = R_CullModel ( model, ent );
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
	fogNum = R_ComputeFogNum( model, ent );

	cubemapIndex = R_CubemapForPoint(ent->e.origin);

	//
	// draw all surfaces
	//
	surface = model->surfaces;
	for ( i = 0 ; i < model->numSurfaces ; i++ ) {

		if ( ent->e.customShader ) {
			shader = R_GetShaderByHandle( ent->e.customShader );
		} else if ( ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins ) {
			skin_t *skin;
			int		j;

			skin = R_GetSkinByHandle( ent->e.customSkin );

			// match the surface name to something in the skin file
			shader = tr.defaultShader;
			for ( j = 0 ; j < skin->numSurfaces ; j++ ) {
				// the names have both been lowercased
				if ( !strcmp( skin->surfaces[j]->name, surface->name ) ) {
					shader = (shader_t *)skin->surfaces[j]->shader;
					break;
				}
			}
			if (shader == tr.defaultShader) {
				ri.Printf( PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surface->name, skin->name);
			}
			else if (shader->defaultShader) {
				ri.Printf( PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name);
			}
		} else {
		    const int skinNum = ent->e.skinNum % surface->numShaderIndexes;
            shader = tr.shaders[surface->shaderIndexes[skinNum]];
		}

		// don't add third_person objects if not viewing through a portal
		if(!personalModel)
		{
			srfVBOMDVMesh_t *vboSurface = &model->vboSurfaces[i];

            auto *vertexAttributes = ojkAllocArray<vertexAttribute_t>(
                *tr.frame.memory, 4);

            const int positionsOffset = 0;
            const int normalsOffset = positionsOffset +
                vboSurface->numVerts * model->numFrames * sizeof(vec3_t);
            const int tangentsOffset = normalsOffset +
                vboSurface->numVerts * model->numFrames * sizeof(uint32_t);
            const int texCoordsOffset = tangentsOffset +
                vboSurface->numVerts * model->numFrames * sizeof(uint32_t);

            vertexAttributes[0].vbo = vboSurface->vbo;
            vertexAttributes[0].index = ATTR_INDEX_POSITION;
            vertexAttributes[0].numComponents = 3;
            vertexAttributes[0].integerAttribute = GL_FALSE;
            vertexAttributes[0].type = GL_FLOAT;
            vertexAttributes[0].normalize = GL_FALSE;
            vertexAttributes[0].stride = 0;
            vertexAttributes[0].offset = positionsOffset;
            vertexAttributes[0].stepRate = 0;

            vertexAttributes[1].vbo = vboSurface->vbo;
            vertexAttributes[1].index = ATTR_INDEX_NORMAL;
            vertexAttributes[1].numComponents = 4;
            vertexAttributes[1].integerAttribute = GL_FALSE;
            vertexAttributes[1].type = GL_UNSIGNED_INT_2_10_10_10_REV;
            vertexAttributes[1].normalize = GL_TRUE;
            vertexAttributes[1].stride = 0;
            vertexAttributes[1].offset = normalsOffset;
            vertexAttributes[1].stepRate = 0;

            vertexAttributes[2].vbo = vboSurface->vbo;
            vertexAttributes[2].index = ATTR_INDEX_TEXCOORD0;
            vertexAttributes[2].numComponents = 2;
            vertexAttributes[2].integerAttribute = GL_FALSE;
            vertexAttributes[2].type = GL_FLOAT;
            vertexAttributes[2].normalize = GL_FALSE;
            vertexAttributes[2].stride = 0;
            vertexAttributes[2].offset = texCoordsOffset;
            vertexAttributes[2].stepRate = 0;

            vertexAttributes[3].vbo = vboSurface->vbo;
            vertexAttributes[3].index = ATTR_INDEX_TANGENT;
            vertexAttributes[3].numComponents = 4;
            vertexAttributes[3].integerAttribute = GL_FALSE;
            vertexAttributes[3].type = GL_UNSIGNED_INT_2_10_10_10_REV;
            vertexAttributes[3].normalize = GL_TRUE;
            vertexAttributes[3].stride = 0;
            vertexAttributes[3].offset = tangentsOffset;
            vertexAttributes[3].stepRate = 0;

            r2::culled_surface_t culledSurface = {};
            culledSurface.entityNum = entityNum;
            culledSurface.shader = shader;

            DrawItem &drawItem = culledSurface.drawItem;
            drawItem = {};
            drawItem.ibo = vboSurface->ibo;
            drawItem.uniformData = nullptr;
            drawItem.attributes = vertexAttributes;
            drawItem.numAttributes = 4;
            drawItem.draw.type = DRAW_COMMAND_INDEXED;
            drawItem.draw.primitiveType = GL_TRIANGLES;
            drawItem.draw.numInstances = 1;
            drawItem.draw.params.indexed.indexType = GL_UNSIGNED_INT;
            drawItem.draw.params.indexed.firstIndex = 0;
            drawItem.draw.params.indexed.numIndices = vboSurface->numIndexes;

            culledSurfaces.push_back(culledSurface);
		}

		surface++;
	}

}





