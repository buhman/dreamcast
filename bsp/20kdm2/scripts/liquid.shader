
textures/liquids/lavahell
{
	// Added to g3map_global texture on May 11, 1999
	q3map_globaltexture
	surfaceparm trans
	//surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 600
	cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}


	
//	END
}
