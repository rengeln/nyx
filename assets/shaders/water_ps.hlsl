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
    float3 screenCoords : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
};

float Fresnel(float NdotL, float fresnelBias, float fresnelPow)

{

  float facing = (1.0 - NdotL);

  return max(fresnelBias +

             (1.0 - fresnelBias) * pow(facing, fresnelPow), 0.0);

}

PS_Out main(PS_In input)
{
    PS_Out output;

    float3 bump = NormalTexture.Sample(LinearSampler, input.worldPosition.xz / 917.0f).xzy * 0.15 +
                  NormalTexture.Sample(LinearSampler, input.worldPosition.xz / 591.0f).xzy * 0.35 +
                  NormalTexture.Sample(LinearSampler, input.worldPosition.xz / 256.0f).xzy * 0.5;
    float3 normal = normalize(float3(0, 0.5f, 0) + (float3(-1.0f, -1.0f, -1.0f) + (bump * 2)));
    float3 reflectionVector = reflect(input.viewVector, normal);
    float fresnel = 0.2f + 0.8f * pow(1.0f - dot(normal, reflectionVector), 2.0f);
    float3 sun = 1.0f * pow(saturate(dot(reflectionVector, LightDirection)), 3.0f);

    float3 reflColor = ReflectionTexture.Sample(LinearSampler, input.screenCoords.xy + normal.xz * 0.05f);

    output.color = float4(lerp(reflColor + sun, float3(0, 0.2f, 0.3f), fresnel), 0.8f);
    return output;
}
