///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "marching_cubes.h"

struct Vertex
{
	float4 pos : SV_Position;
	uint id : TEXCOORD0;
};

//
//  Passthrough pixel shader for the splat vertices phase.
//
uint main(Vertex v) : SV_Target
{
	return v.id;
}



