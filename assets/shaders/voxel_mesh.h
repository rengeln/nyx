///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

cbuffer SceneConstants {
	row_major float4x4 ProjectionViewMatrix;
	row_major float4x4 WorldMatrix;
    float Transparency;
};

sampler MaterialSampler;
texture2D MaterialTextures[4];

static const float3 LightDirection = {0.5f, -0.5, 0.5f};
static const float3 LightColor = {0.7f, 0.7f, 0.7f};
static const float3 AmbientColor = {0.1f, 0.1f, 0.1f};

//
//  Mapping of material IDs to textures in the X, Y, and Z projections.
//
static const uint3 MaterialToTexture[8] =
{
	{0, 0, 0},
	{0, 1, 0},
	{0, 2, 0},
	{0, 3, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0}
};