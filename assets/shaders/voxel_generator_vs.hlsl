///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

struct Input
{
    uint unused : POSITION;
    uint id : SV_InstanceID;
};

struct Output
{
    float3 pos : POSITION;
    float3 cellpos : TEXCOORD0;
};

static const uint3 Dimensions = uint3(32, 32, 32);
static const float3 DimensionsF = float3(32.0f, 32.0f, 32.0f);

//
//  Vertex shader for the voxel generator.
//
//  Each vertex corresponds to a voxel. To avoid needing to send a vertex buffer
//  consisting of nothing but "1, 2, 3, 4, ..., N." we take advantage of instancing;
//  only one vertex of data is sent N times, and we look at the InstanceID. A
//  minor optimization but a simple one.
//
Output main(Input input)
{
    //  Calculate the cell position in voxel space.
	uint3 cpos;
	cpos.x = input.id % Dimensions.x;
	cpos.y = (input.id % (Dimensions.x * Dimensions.y)) / Dimensions.x;
	cpos.z = input.id / (Dimensions.x * Dimensions.y); 

    //  Calculate the cell position in world space.
    Output output;
    output.pos.xy = -1.0f + (((float2)cpos.xy / DimensionsF.xy) * 2.0f);
    output.pos.xy += 1.0f / DimensionsF.xy;
    output.pos.y = -output.pos.y;
    output.pos.z = cpos.z;
    output.cellpos = (float3)cpos;
    return output;
}

