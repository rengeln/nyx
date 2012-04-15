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
    float3 screenCoords : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
};

//
//  Vertex shader for rendering lines.
//
VS_Out main(VS_In input)
{
    VS_Out output;

    float3 pos = float3(input.position.x + CameraPos.x,
                        input.position.y,
                        input.position.z + CameraPos.z);
    output.worldPosition = pos;
    output.viewVector = normalize(pos - CameraPos);
    output.position = mul(float4(pos, 1.0f), mul(ViewMatrix, ProjectionMatrix));
    output.screenCoords.x = (1.0f + (output.position.x / output.position.w)) * 0.5f;
    output.screenCoords.y = 1.0f - ((1.0f + (output.position.y / output.position.w)) * 0.5f);
    output.screenCoords.z = output.position.z;
    return output; 
}
