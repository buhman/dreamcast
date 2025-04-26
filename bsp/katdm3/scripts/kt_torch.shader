//kt_torch md3 map model (c) ken 'kat' beyer
//script modified from id original


//torch with no flame
models/mapobjects/kt_torch/kt_torch
{
    cull disable
    
        {
                map models/mapobjects/kt_torch/kt_torch.tga
                alphaFunc GE128
		depthWrite
		rgbGen vertex
        }


}

//torch with yellow flame
models/mapobjects/kt_torch/kt_torch_flame
{
    cull disable
    
        {
                map models/mapobjects/kt_torch/kt_torch.tga
                alphaFunc GE128
		depthWrite
		rgbGen vertex
        }


}

//torch with smoke plume
models/mapobjects/kt_torch/kt_torch_smoke
{
    cull disable
    
        {
                map models/mapobjects/kt_torch/kt_torch.tga
                alphaFunc GE128
		depthWrite
		rgbGen vertex
        }


}

//kt_smoke - modified from id original script

textures/kt_misc/kt_smoke
{

	surfaceparm nomarks
	surfaceparm nolightmap
	cull none
	

	{
		animMap 10 textures/kt_misc/smoke1.jpg textures/kt_misc/smoke2.jpg textures/kt_misc/smoke3.jpg textures/kt_misc/smoke4.jpg textures/kt_misc/smoke5.jpg textures/kt_misc/smoke6.jpg textures/kt_misc/smoke7.jpg textures/kt_misc/smoke8.jpg
		blendFunc GL_ONE GL_ONE
		rgbGen wave inverseSawtooth 0 1 0 10	
	}
	
	{
		animMap 10 textures/kt_misc/smoke2.jpg textures/kt_misc/smoke3.jpg textures/kt_misc/smoke4.jpg textures/kt_misc/smoke5.jpg textures/kt_misc/smoke6.jpg textures/kt_misc/smoke7.jpg textures/kt_misc/smoke8.jpg textures/kt_misc/smoke1.jpg 
		blendFunc GL_ONE GL_ONE
		rgbGen wave sawtooth 0 1 0 10
	}	

}
