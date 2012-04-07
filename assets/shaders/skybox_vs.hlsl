///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

cbuffer SceneConstants {
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
};

struct VS_In {
	float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_Out {
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
};

//
//  Vertex shader for skybox rendering.
//
VS_Out main(VS_In input)
{
	VS_Out output;

    float3 rotatedPosition = mul(input.position, ViewMatrix);
    output.position = mul(float4(rotatedPosition, 1.0f), ProjectionMatrix).xyww;
    output.uv = input.uv;

	return output; 
}
