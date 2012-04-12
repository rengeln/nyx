///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

cbuffer SceneConstants {
    row_major float4x4 ProjectionViewMatrix;
};

struct VS_In {
	float3 position : POSITION;
};

struct VS_Out {
	float4 position : SV_Position;
};

//
//  Vertex shader for rendering lines.
//
VS_Out main(VS_In input)
{
	VS_Out output;
    output.position = mul(float4(input.position, 1.0f), ProjectionViewMatrix);
	return output; 
}
