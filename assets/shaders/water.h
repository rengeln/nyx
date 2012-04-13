///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

cbuffer SceneConstants {
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    float4 CameraPosition;
    float2 Offset;
};

texture2D<float> NoiseTexture;
sampler NoiseSampler;

static const float3 LightDirection = {0.5f, -0.5, 0.5f};
static const float3 LightColor = {0.8f, 0.6f, 0.4f};
static const float3 AmbientColor = {0.0f, 0.2f, 0.2f};

float Displacement(float2 xy)
{
    float y = 0;
    y += NoiseTexture.SampleLevel(NoiseSampler, xy / 7135.0f, 0) * 0.5;
    y += NoiseTexture.SampleLevel(NoiseSampler, xy / 3173.0f, 0) * 0.35;
    y += NoiseTexture.SampleLevel(NoiseSampler, xy / 173.0f, 0) * 0.15;
    return y;
}

