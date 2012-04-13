///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "voxel_mesh.h"

struct PS_Out {
	float4 color : SV_Target;
};

struct PS_In {
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 worldPos : TEXCOORD0;
	float4 materialWeights[2] : TEXCOORD1;
    float fog : TEXCOORD3;
};

//
//  Pixel shader for rendering voxel meshes.
//
PS_Out main(PS_In input)
{
    //
    //  Calculate the texture weights in each dimension.
    //
    float3 blend_weights = abs(input.normal.xyz);  
    blend_weights.y *= 0.4f;
    blend_weights = (blend_weights - 0.2) * 7;  
    blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)  
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z ).xxx;   

    //
    //  Calculate the texture coordinates.
    //
    //  To avoid artifacts caused by repeating each texture is sampled twice at non-synchronous
    //  repeat values, then blended together.
    //
    float2 coord1 = input.worldPos.zy / 12.131f;  
    float2 coord2 = input.worldPos.zx / 11.371f;  
    float2 coord3 = input.worldPos.xy / 13.129f; 

	float2 coord4 = input.worldPos.zy / 253.189f;
	float2 coord5 = input.worldPos.zx / 271.47f;
	float2 coord6 = input.worldPos.xy / 233.731f;

	float3 blendColor = float3(0, 0, 0);
	float materialWeights[8] =
	{
		input.materialWeights[0].x,
		input.materialWeights[0].y,
		input.materialWeights[0].z,
		input.materialWeights[0].w,
		input.materialWeights[1].x,
		input.materialWeights[1].y,
		input.materialWeights[1].z,
		input.materialWeights[1].w
	};

    //
    //  Note: I experimented with calculating the ddx, ddy, ddz outside of the loop and then
    //  using SampleGrad, but there was no performance benefit.
    //
	[unroll] for (uint i = 0; i < 7; i++)
	{
		uint3 index = MaterialToTexture[i];
		float3 color = MaterialTextures[index.x].Sample(MaterialSampler, coord1).xyz * blend_weights.xxx +
					   MaterialTextures[index.y].Sample(MaterialSampler, coord2).xyz * blend_weights.yyy +
					   MaterialTextures[index.z].Sample(MaterialSampler, coord3).xyz * blend_weights.zzz;
		blendColor += color * (materialWeights[i] * 1.0f);
			
		color = MaterialTextures[index.x].Sample(MaterialSampler, coord4).xyz * blend_weights.xxx +
				MaterialTextures[index.y].Sample(MaterialSampler, coord5).xyz * blend_weights.yyy +
				MaterialTextures[index.z].Sample(MaterialSampler, coord6).xyz * blend_weights.zzz;
		blendColor += color * (materialWeights[i] * 0.5f);
    }

    //
    //  Extremely simple lighting calculation.
    //
	float3 diffuse = LightColor * saturate(dot(input.normal, LightDirection));
	
    //
    //  Put it all together.
    //
	PS_Out output;
	output.color = float4(blendColor * saturate(AmbientColor + diffuse), Transparency);

	return output;
}
