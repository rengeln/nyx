///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "voxel_mesh.h"

struct VS_In {
	float3 position : POSITION;
	uint normal : NORMAL;
	uint2 material : TEXCOORD0;
};

struct VS_Out {
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 worldPos : TEXCOORD0;
	float4 materialWeights[2] : TEXCOORD1;
};

//
//  Vertex shader for rendering voxel meshes.
//
//  To save memory the vertex format is compacted, so this performs the unpacking
//  before passing the values to the pixel shader. Both the normal and material
//  weights are packed.
//
VS_Out main(VS_In input)
{
	VS_Out output;

	float4x4 finalMatrix = mul(WorldMatrix, ProjectionViewMatrix);
	
	float3 pos = input.position.xyz;
	output.position = mul(float4(pos, 1.0f), finalMatrix);

	uint3 unpackedNormal = uint3((input.normal >> 24) & 0xFF, 
								 (input.normal >> 16) & 0xFF, 
								  input.normal & 0xFF);
	output.normal = normalize((float3(unpackedNormal) - 127.0f) / 128.0f);
	output.worldPos = input.position.xyz;
	output.materialWeights[0] = float4(
		float((input.material.x >> 24) & 0xFF) / 255.0f,
		float((input.material.x >> 16) & 0xFF) / 255.0f,
		float((input.material.x >> 8) & 0xFF) / 255.0f,
		float((input.material.x >> 0) & 0xFF) / 255.0f);
	output.materialWeights[1] = float4(
		float((input.material.y >> 24) & 0xFF) / 255.0f,
		float((input.material.y >> 16) & 0xFF) / 255.0f,
		float((input.material.y >> 8) & 0xFF) / 255.0f,
		float((input.material.y >> 0) & 0xFF) / 255.0f);
	return output; 
}
