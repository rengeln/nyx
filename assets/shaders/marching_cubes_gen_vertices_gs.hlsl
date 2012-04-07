///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

struct Vertex
{
	float3 position : POSITION;
	uint normal : NORMAL;
    uint2 material : TEXCOORD0;
};

//
//  Passthrough geometry shader for the vertex-generation phase.
//
[maxvertexcount(1)]
void main(point Vertex v[1], inout PointStream<Vertex> output)
{
	output.Append(v[0]);
}

