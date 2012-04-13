///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "water.h"

struct VS_In {
	float3 position : POSITION;
};

struct VS_Out {
	float4 position : SV_Position;
    float3 worldPosition : TEXCOORD0;
};

//
//  Vertex shader for rendering lines.
//
VS_Out main(VS_In input)
{
    VS_Out output;

    output.worldPosition = input.position;
    output.worldPosition.x += CameraPosition.x;
    output.worldPosition.z += CameraPosition.z;

    input.position.x += CameraPosition.x;
    input.position.z += CameraPosition.z;
    input.position.y += Displacement(output.worldPosition.xz) * 2.0f;
    output.position = mul(float4(input.position, 1.0f), mul(ViewMatrix, ProjectionMatrix));
    
    return output; 
}
