models/mapobjects/timlamp/timlamp
{
	cull disable
	surfaceparm alphashadow
	{
		map models/mapobjects/timlamp/timlamp.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
}

models/mapobjects/gratelamp/gratetorch2b
{
	cull disable
	{
		map models/mapobjects/gratelamp/gratetorch2b.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}

}

models/mapobjects/wallhead/lion_m
{
	{
		map models/mapobjects/wallhead/lion_m.tga
		blendFunc GL_ONE GL_ZERO
		rgbGen vertex
	}
	{
		map textures/sfx/firewalla.tga
		blendFunc GL_ONE GL_ONE
		tcmod scroll 0.1 1
		//rgbGen wave triangle .5 1 0 .4
	}
	{
		map models/mapobjects/wallhead/lion_m.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}

