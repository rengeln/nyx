///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "marching_cubes.h"

struct Marker
{
	//	x : 8
	//	y : 8
	//  z : 8
	//  edge: 8
	uint packed : POSITION;
	uint id : SV_VertexID;
};

struct Vertex
{
	float4 pos : SV_Position;
	uint id : TEXCOORD0;
};

//
//  Vertex shader for the splat vertices phase.
//
//  Given a vertex marker produced by the list vertices phase, this will 
//  output the associated index into the lookup texture. 
//  
Vertex main(Marker marker)
{
	//
	//	Unpack the cell position and edge number.
	//
	uint3 cellPos;
	uint edge;
	cellPos.x = (marker.packed >> 24) & 0xFF;
	cellPos.y = (marker.packed >> 16) & 0xFF;
	cellPos.z = (marker.packed >> 8) & 0xFF;
	edge = marker.packed & 0xFF;
	
    //  
    //  Translate from voxel space to lookup texture space.
    //
	cellPos.x *= 3;
    cellPos += EdgeXOffsets[edge];

	Vertex v;
	v.pos = float4(0, 0, 0, 1.0f);
	v.pos.y = ((cellPos.z * (float)CellCount.y) + cellPos.y) / (float)(CellCount.y * CellCount.z);
	v.pos.x = (cellPos.x / ((float)CellCount.x * 3.0f));
    
    v.pos.x += (1 / ((float)CellCount.x * 3.0f));
    v.pos.y += (1 / ((float)(CellCount.y * CellCount.z)));

	v.pos.x = -1.0f + v.pos.x * 2.0f;
	v.pos.y = -1.0f + v.pos.y * 2.0f;

	v.pos.y = -v.pos.y;

    //  Assign the index value
	v.id = marker.id;

	return v;
}

