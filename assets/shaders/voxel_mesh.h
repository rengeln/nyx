///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

cbuffer SceneConstants {
	row_major float4x4 ProjectionViewMatrix;
	row_major float4x4 WorldMatrix;
    float Transparency;
    float4 ClipPlane;
};

sampler MaterialSampler;
texture2D MaterialTextures[8];

static const float3 LightDirection = {0.5f, -0.5, 0.5f};
static const float3 LightColor = {0.8f, 0.6f, 0.4f};
static const float3 AmbientColor = {0.0f, 0.2f, 0.2f};

//
//  Mapping of material IDs to textures in the X, Y, and Z projections.
//
static const uint3 MaterialToTexture[8] =
{
	{3, 3, 3},          //  DIRT
	{4, 1, 4},          //  GRASS (dark)
	{4, 2, 4},          //  GRASS (light)
	{4, 3, 4},          //  ROCK (dark)
	{5, 3, 5},          //  ROCK (light)
	{0, 6, 0},          //  SAND
	{0, 0, 0},
	{0, 0, 0}
};