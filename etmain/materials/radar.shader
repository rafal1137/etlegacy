// radar.shader

textures/radar/dirt_m03icmp_brown
{
	qer_editorimage textures/temperate_sd/dirt_m03icmp_brown.tga
	diffuseMap textures/radar/dirt_m03icmp_brown.tga
	bumpMap textures/radar/dirt_m03icmp_brown_n.tga
	specularMap textures/radar/dirt_m03icmp_brown_r.tga
	cull twosided
	surfaceparm trans
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}    

textures/radar/dirt_m04cmp_brown
{
	qer_editorimage textures/temperate_sd/dirt_m04cmp_brown.tga
	diffuseMap textures/radar/dirt_m04cmp_brown.tga
	bumpMap textures/radar/dirt_m04cmp_brown_n.tga
	specularMap textures/radar/dirt_m04cmp_brown_r.tga
	cull twosided
	surfaceparm trans
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}

}
// could be fun to make this move... thoughts for later
textures/radar/fog
{
	qer_editorimage textures/sfx/fog_grey1.tga
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm fog
//	fogparms ( 0.09411 0.09803 0.12549 ) 3192
	fogparms ( 0.10588 0.11765 0.14510 ) 20000
}

textures/radar/lmterrain2_base
{
	q3map_normalimage textures/sd_bumpMaps/normalmap_terrain.tga
	q3map_lightmapsize 512 512
	q3map_lightmapMergable
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 512 0 0 ) ( 0 512 0 )
	q3map_tcMod scale 2 2
	q3map_tcMod rotate 37
	surfaceparm grasssteps
	surfaceparm landmine	
}

textures/radar/lmterrain2_foliage_base
{
	q3map_baseShader textures/radar/lmterrain2_base
	q3map_foliage models/foliage/grassfoliage1.md3 1.25 48 0.1 2
	q3map_foliage models/foliage/grassfoliage2.md3 1.1 48 0.1 2
	q3map_foliage models/foliage/grassfoliage3.md3 1 48 0.1 2
}

textures/radar/lmterrain2_foliage_fade
{
	q3map_baseShader textures/radar/lmterrain2_base
	q3map_foliage models/foliage/grassfoliage1.md3 1.25 64 0.1 2
	q3map_foliage models/foliage/grassfoliage2.md3 1.1 64 0.1 2
	q3map_foliage models/foliage/grassfoliage3.md3 1 64 0.1 2
}

textures/radar/lmterrain2_0
{
	q3map_baseshader textures/radar/lmterrain2_base
/*
	diffuseMap textures/temperate_sd/master_grass_dirt3.tga
	bumpMap textures/temperate_sd/master_grass_dirt3_n.tga
	specularMap textures/temperate_sd/master_grass_dirt3_r.tga
*/
	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_1
{
	q3map_baseshader textures/radar/lmterrain2_foliage_base
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_path1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_path1_r.tga
		rgbgen identity
	}
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_path1_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_path1_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_2
{
	q3map_baseshader textures/radar/lmterrain2_base
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		rgbgen identity
	}
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_3
{
	q3map_baseshader textures/radar/lmterrain2_base
    {
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_4
{
	q3map_baseshader textures/radar/lmterrain2_base
   
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/dirt3_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/dirt3_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_5
{
	q3map_baseshader textures/radar/lmterrain2_base
    {
	   stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		rgbgen identity
	}
	{   stage bumpMap
		map textures/temperate_sd/grass_ml03cmp_n.tga
		rgbgen identity
	}
	{   stage specularMap
		map textures/temperate_sd/grass_ml03cmp_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/grass_ml03cmp_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/grass_ml03cmp_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_0to1
{
	q3map_baseshader textures/radar/lmterrain2_foliage_fade

	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/grass_path1_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/grass_path1_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_0to2
{
	q3map_baseshader textures/radar/lmterrain2_base

	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_0to3
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_0to4
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/dirt3_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/dirt3_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}
textures/radar/lmterrain2_0to5
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/master_grass_dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/master_grass_dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/master_grass_dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_ml03cmp_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_ml03cmp_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_1to2
{
	q3map_baseshader textures/radar/lmterrain2_foliage_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/grass_path1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/grass_path1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_1to3
{
	q3map_baseshader textures/radar/lmterrain2_foliage_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/grass_path1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/grass_path1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_1to4
{
	q3map_baseshader textures/radar/lmterrain2_foliage_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/grass_path1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/grass_path1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/dirt3_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/dirt3_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_1to5
{
	q3map_baseshader textures/radar/lmterrain2_foliage_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_path1.tga
		rgbgen identity
	}
	{   stage bumpMap
	    map textures/temperate_sd/grass_path1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
	    map textures/temperate_sd/grass_path1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_ml03cmp_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_ml03cmp_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_2to3
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		rgbgen identity
		
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_2to4
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/dirt3_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/dirt3_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_2to5
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_dense1.tga
		rgbgen identity
		
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_dense1_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_dense1_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_ml03cmp_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_ml03cmp_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_3to4
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/dirt3_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/dirt3_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_3to5
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/rock_ugly_brown.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/rock_ugly_brown_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/rock_ugly_brown_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_ml03cmp_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_ml03cmp_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/lmterrain2_4to5
{
	q3map_baseshader textures/radar/lmterrain2_base
	
	{
	    stage diffuseMap
		map textures/temperate_sd/dirt3.tga
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/dirt3_n.tga
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/dirt3_r.tga
		rgbgen identity
	}
	{
	    stage diffuseMap
		map textures/temperate_sd/grass_ml03cmp.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/grass_ml03cmp_n.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/grass_ml03cmp_r.tga
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

//**********************************************
// rain FX
//base shader has problem with texture scaling
//should be fixed by me or ydnar later
//back to the old school way - Chris-
//**********************************************

textures/radar/road_wet
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/temperate_sd/dirt_m03icmp_brown.tga
	q3map_nonplanar
	q3map_shadeangle 120
	surfaceparm trans
	{
	    stage diffuseMap
	    Map textures/temperate_sd/dirt_m03icmp_brown.tga
		rgbGen identity
	}
	{
	    stage bumpMap
	    Map textures/temperate_sd/dirt_m03icmp_brown_n.tga
		rgbGen identity
	}
	{
	    stage specularMap
	    Map textures/temperate_sd/dirt_m03icmp_brown_r.tga
		rgbGen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/road
{
	qer_editorimage textures/temperate_sd/dirt_m03icmp_brown.tga
	q3map_nonplanar
	q3map_shadeangle 120
	surfaceparm trans
	{
		stage diffuseMap 
		map textures/temperate_sd/dirt_m03icmp_brown.tga
		rgbGen identity	
	}
	{	
		stage bumpMap
		map textures/temperate_sd/dirt_m03icmp_brown_n.tga
		rgbGen identity	
	}
	{
		stage specularMap
		map textures/temperate_sd/dirt_m03icmp_brown_r.tga
		rgbGen identity	
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}


textures/radar/road_puddle1
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/temperate_sd/road_puddle1.tga
	q3map_nonplanar
	q3map_shadeangle 120
	surfaceparm trans
	surfaceparm splash
	diffuseMap textures/temperate_sd/road_puddle1.tga
	bumpMap textures/temperate_sd/road_puddle1_n.tga
	specularMap textures/temperate_sd/road_puddle1_r.tga
/*		
	{
	    stage diffuseMap
		map textures/temperate_sd/road_puddle1.tga
//		map textures/temperate_sd/dirt_m03icmp_brown.tga
		blendFunc blend
//		depthFunc lequal
		rgbGen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/road_puddle1_n.tga
//		map textures/temperate_sd/dirt_m03icmp_brown_n.tga
//		blendFunc blend
//		depthFunc lequal
		rgbGen identity
	}
	{   
	    stage specularMap
		map textures/temperate_sd/road_puddle1_r.tga
//		blendFunc blend
//		depthFunc lequal
		rgbGen identity
	}
*/
//	{
//		map textures/effects/envmap_radar.tga
//		tcGen environment
//		depthFunc lequal
//		rgbGen identity
//	}
/*
	{
		map textures/liquids_sd/puddle_specular.tga
		tcGen environment
		blendFunc GL_SRC_ALPHA GL_ONE
		depthFunc lequal
		rgbGen identity
	}
*/
	{
		lightmap $lightmap
		blendFunc filter
		rgbgen identity
	}
}

textures/radar/road_bigpuddle
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/temperate_sd/road_bigpuddle.tga
	q3map_nonplanar
	q3map_shadeangle 120
	surfaceparm trans
	surfaceparm splash
	diffusemap textures/temperate_sd/road_bigpuddle.tga
	bumpMap textures/temperate_sd/road_bigpuddle_n.tga
	specularMap textures/temperate_sd/road_bigpuddle_r.tga
//	reflectionMap $whiteimage
/*
	{
	    stage diffuseMap
		map textures/temperate_sd/road_bigpuddle.tga
		blendFunc blend
//		depthFunc lequal
//		rgbGen identity
	}
	{
	    stage bumpMap
		map textures/temperate_sd/road_bigpuddle_n.tga
//		blendFunc blend
//		depthFunc lequal
//		rgbGen identity
	}
	{
	    stage specularMap
		map textures/temperate_sd/road_bigpuddle_r.tga
//		blendFunc blend
//		depthFunc lequal
//		rgbGen identity
	}
*/
/*
	{
		map textures/effects/envmap_radar.tga
		rgbGen identity
		alphaGen const 0.5
		blendFunc blend
//		tcMod scale 0.5 0.5
		tcGen environment
		depthFunc lequal
	}
*/
/*
	{
		map textures/liquids_sd/puddle_specular.tga
		rgbGen identity
		tcMod scale 2 2
	    blendFunc blend //GL_SRC_ALPHA GL_ONE
		depthFunc lequal
		tcGen environment
	}
*/
	{
		lightmap $lightmap
		blendFunc filter
		rgbgen identity
	}
}


textures/radar/borderroad
{
	qer_editorimage textures/temperate_sd/dirt_m04cmp_brown.tga
	surfaceparm trans
//	diffuseMap textures/temperate_sd/dirt_m04cmp_brown.tga
	bumpMap textures/temperate_sd/dirt_m04cmp_brown_n.tga
	specularMap textures/temperate_sd/dirt_m04cmp_brown_r.tga

	{
	    stage diffuseMap
		map textures/temperate_sd/dirt_m04cmp_brown.tga
//		blendFunc blend
		rgbGen identity
	}

	{
		lightmap $lightmap
		blendFunc filter
		rgbgen identity
	}
}

textures/radar/wood_m02_wet
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/wood/wood_m02.tga
	diffuseMap textures/wood/wood_m02.tga
	bumpMap textures/wood/wood_m02_n.tga
	specularMap textures/wood/wood_m02_r.tga
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/gy_ml03a_wet
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/graveyard/gy_ml03a.tga
	diffuseMap textures/graveyard/gy_ml03a.tga
	bumpMap textures/graveyard/gy_ml03a_n.tga
	specularMap textures/graveyard/gy_ml03a_r.tga
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}

}

textures/radar/debri_m05_wet
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/rubble/debri_m05.tga
	diffuseMap textures/rubble/debri_m05.tga
	bumpMap textures/rubble/debri_m05_n.tga
	specularMap textures/rubble/debri_m05_r.tga
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/wood_m16_wet
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/wood/wood_m16.tga
	diffuseMap textures/wood/wood_m16.tga
	bumpMap textures/wood/wood_m16_n.tga
	specularMap textures/wood/wood_m16_r.tga
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/wall_c01_wet
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/sleepy/wall_c01.tga
	diffuseMap textures/sleepy/wall_c01.tga
	bumpMap textures/sleepy/wall_c01_n.tga
	specularMap textures/sleepy/wall_c01_r.tga
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/metal_wet1
{
	qer_editorimage textures/metals_sd/metal_ref1.tga
	{
		map textures/effects/envmap_radar.tga
		rgbGen identity
		tcMod scale 0.5 0.5
		tcGen environment
	}
	{
		map textures/liquids_sd/puddle_specular.tga
		rgbGen identity
		tcMod scale 2 2
		blendFunc GL_SRC_ALPHA GL_ONE
		tcGen environment
	}
	{
		map textures/metals_sd/metal_ref1.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}
}

textures/radar/metal_wet2
{
//	q3map_foliage models/foliage/raincircle0.md3 1 64 0.1 2
//	q3map_foliage models/foliage/raincircle1.md3 0.8 64 0.1 2
//	q3map_foliage models/foliage/raincircle2.md3 0.6 64 0.1 2
//	q3map_foliage models/foliage/raincircle3.md3 0.9 64 0.1 2
//	q3map_foliage models/foliage/raincircle4.md3 0.7 64 0.1 2
//	q3map_foliage models/foliage/raincircle5.md3 0.5 64 0.1 2
	qer_editorimage textures/metals_sd/metal_ref1.tga
	{
		map textures/effects/envmap_radar.tga
		rgbGen identity
		tcMod scale 0.5 0.5
		tcGen environment
	}
	{
		map textures/liquids_sd/puddle_specular.tga
		rgbGen identity
		tcMod scale 2 2
		blendFunc  GL_SRC_ALPHA GL_SRC_COLOR
		tcGen environment
	}
	{
		map textures/metals_sd/metal_ref1.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen identity
	}
	{
		lightmap $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbgen identity
	}

}
