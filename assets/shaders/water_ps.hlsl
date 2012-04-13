///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "water.h"

struct PS_Out {
	float4 color : SV_Target;
};

struct PS_In {
	float4 position : SV_Position;
    float3 worldPosition : TEXCOORD0;
};

//
//  Pass-through pixel shader for rendering lines.
//
PS_Out main(PS_In input)
{
    PS_Out output;

    //  Calculate the slope in the x and z directions to produce a normal
    float d = Displacement(input.worldPosition.xz);
    float dx = Displacement(float2(input.worldPosition.x + 10.0f,
                                   input.worldPosition.z));
    float dz = Displacement(float2(input.worldPosition.x,
                                   input.worldPosition.z + 10.0f));
    float3 normal = normalize(float3(d - dx, 0.1f, d - dz));
    //output.color = float4(normal, 1.0f);
    
    output.color = pow(pow(saturate(dot(R, i.sun)),sun_shininess)*float3(1.2, 0.4, 0.1), 1/2.2);
    /*
    float d = Displacement(input.worldPosition.xz);
    output.color = float4(d, 0, 0, 1.0f);
    */
    //output.color = float4(0.0f, 0.1f, 0.3f, 0.7f);
    return output;
}
