// models_weapons2.shader
// 16 total shaders

/* r1 shaders start at line 141
models/weapons2/c4_dynamite/dynomite1a
{
	 diffusemap models/weapons2/c4_dynamite/dynomite1a
	 specularmap models/weapons2/c4_dynamite/dynomite1a_s
	 bumpmap models/weapons2/c4_dynamite/dynomite1a_n
}

models/weapons2/grenade/grenade_us
{
	 diffusemap models/weapons2/grenade/grenade_us
	 specularmap models/weapons2/grenade/grenade_us_s
	 bumpmap models/weapons2/grenade/grenade_us_n
}

models/weapons2/colt/colt4
{
	 diffusemap models/weapons2/colt/colt_yd
	 bumpmap models/weapons2/colt/colt_yd_n
	 specularmap models/weapons2/colt/colt_yd_s
}

models/weapons2/fg42/fg42_2
{
     diffusemap models/weapons2/fg42/fg42_yd
     bumpmap models/weapons2/fg42/fg42_yd_n	
	 specularmap models/weapons2/fg42/fg42_yd_s
}

models/weapons2/flamethrower/flame1
{
	 diffusemap models/weapons2/flamethrower/flame1_yd
	 bumpmap models/weapons2/flamethrower/flame1_yd_n
	 specularmap models/weapons2/flamethrower/flame1_yd_s
}

models/weapons2/grenade/grenade
{
	 diffusemap models/weapons2/grenade/grenade_yd
	 bumpmap models/weapons2/grenade/grenade_yd_n
     specularmap models/weapons2/grenade/grenade_yd_s
}

models/weapons2/knife/knife1a
{
	 diffusemap models/weapons2/knife/knife_yd
	 bumpmap models/weapons2/knife/knife_yd_n
	 specularmap models/weapons2/knife/knife_yd_s
}

//flamethrower flash
models/weapons2/lightning/f_lightning
{
	cull disable
	sort additive
	{
		map models/weapons2/flamethrower/flash.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/luger/luger7
{
	 diffusemap models/weapons2/luger/luger7_yd
	 bumpmap models/weapons2/luger/luger7_yd_n
	 specularmap models/weapons2/luger/luger7_yd_s
}

//This is the same as R1 its the flash for smg's and mg's
models/weapons2/machinegun/f_machinegun
{
	cull disable
	nofog
	sort additive
	{
		animMap 2 models/weapons2/machinegun/f_machinegun.tga models/weapons2/machinegun/f_machinegun1.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/mauser/mauser3
{
	 diffusemap models/weapons2/mauser/mauser3_yd
	 bumpmap models/weapons2/mauser/mauser3_yd_n
	 specularmap models/weapons2/mauser/mauser3_yd_s
	
}

models/weapons2/mp40/gun11
{
	 diffusemap models/weapons2/mp40/gun11_yd
	 bumpmap models/weapons2/mp40/gun11_yd_n
	 specularmap models/weapons2/mp40/gun11_yd_s
}

//rocket flare?
models/weapons2/rocketl/f_rocketl
{
	cull disable
	sort additive
	{
		map models/weapons2/rocketl/f_rocketl.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/rocketl/panzerfast1a
{
	 diffusemap models/weapons2/panzerfaust/panzerfaust_yd
	 bumpmap models/weapons2/panzerfaust/panzerfaust_yd_n
	 specularmap models/weapons2/panzerfaust/panzerfaust_yd_s
}

models/weapons2/rocketl/rocketl14
{
	cull disable
	sort additive
	{
		map models/weapons2/rocketl/f_rocketl.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/sten/sten1
{
	 diffusemap models/weapons2/sten/sten1_yd
	 bumpmap models/weapons2/sten/sten1_yd_n
	 specularmap models/weapons2/sten/sten1_yd_s	
}

models/weapons2/thompson/thompson_la
{
	 diffusemap models/weapons2/thompson/thompson_la_yd
	 bumpmap models/weapons2/thompson/thompson_la_yd_n
     specularmap models/weapons2/thompson/thompson_la_yd_s	
}
*/

// models_weapons2.shader
// generated by ShaderCleaner on Thu Feb  6 12:43:32 2003
// 16 total shaders


models/weapons2/c4_dynamite/dynomite1a
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/c4_dynamite/dynomite1a.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/grenade/grenade_us
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/grenade/grenade_us.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}


models/weapons2/colt/colt4
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/colt/colt_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/fg42/fg42_2
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/fg42/fg42_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/flamethrower/flame1
{
/*
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/flamethrower/flame1_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
*/
{
stage diffuseMap
map models/weapons2/flamethrower/flame1_yd.tga
}
bumpMap models/weapons2/flamethrower/flame1_yd_n.tga
specularMap models/weapons2/flamethrower/flame1_yd_x.tga // this specmap _r is no good (check: if specmap is low, then also not much reflections?)
reflectionMap models/weapons2/flamethrower/flame1_yd_x.tga
}

models/weapons2/grenade/grenade
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/grenade/grenade_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/knife/knife1a
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/knife/knife_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/lightning/f_lightning
{
	cull disable
	sort additive
	{
		map models/weapons2/flamethrower/flash.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/luger/luger7
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/luger/luger7_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/machinegun/f_machinegun
{
	cull disable
	nofog
	sort additive
	{
		animMap 2 models/weapons2/machinegun/f_machinegun.tga models/weapons2/machinegun/f_machinegun1.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/mauser/mauser3
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/mauser/mauser3_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/mp40/gun11
{
/*
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/mp40/gun11_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
*/
diffuseMap models/weapons2/mp40/gun11_yd.tga
bumpMap models/weapons2/mp40/gun11_yd_n.tga
specularMap models/weapons2/mp40/gun11_yd_r.tga
reflectionMap models/weapons2/mp40/gun11_yd_x.tga
}

models/weapons2/rocketl/f_rocketl
{
	cull disable
	sort additive
	{
		map models/weapons2/rocketl/f_rocketl.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/rocketl/panzerfast1a
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/panzerfaust/panzerfaust_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/rocketl/rocketl14
{
	cull disable
	sort additive
	{
		map models/weapons2/rocketl/f_rocketl.tga
		blendfunc GL_ONE GL_ONE
	}
}

models/weapons2/sten/sten1
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/sten/sten1_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/thompson/thompson_la
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/thompson/thompson_la_yd.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}
