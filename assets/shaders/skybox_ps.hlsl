///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

struct PS_Out {
	float4 color : SV_Target;
};

struct PS_In {
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
};

sampler SkySampler;
texture2D SkyTexture;

//
//  Pixel shader for skybox rendering.
//
PS_Out main(PS_In input)
{
    PS_Out output;
    output.color = SkyTexture.Sample(SkySampler, input.uv);
    return output;
}
