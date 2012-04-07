///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

struct Input
{
    float3 pos : POSITION;
    float3 cellpos : TEXCOORD0;
};

struct Output
{
    float4 pos : SV_Position;
    float3 cellpos : TEXCOORD0;
    uint slice : SV_RenderTargetArrayIndex;
};

//
//  Geometry shader for the voxel generator.
//
//  Just about the only thing this does is mapping cellpos.z to
//  RenderTargetArrayIndex in order to output to a 3D texture.
//
[maxvertexcount(1)]
void main(point Input input[1], inout PointStream<Output> stream)
{
    Output output;
    output.pos = float4(input[0].pos.xy, 0.0f, 1.0f);
    output.slice = (uint)input[0].pos.z;
    output.cellpos = input[0].cellpos;
    stream.Append(output);
}

