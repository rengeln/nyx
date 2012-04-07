///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "marching_cubes.h"

struct Cell
{
    //  x : 8
    //  y : 8
    //  z : 8
    //  cornerMask: 8
	uint id : POSITION;
};

struct Marker
{
	//	x : 8
	//	y : 8
	//  z : 8
	//  edge: 8
	uint packed : POSITION;
};

//
//  Geometry shader for the list-vertices phase.
//
//  This takes as input the informtion for an individual cell and outputs
//  a marker for each vertex which needs to be produced for that cell.
//
[maxvertexcount(4)]
void main(point Cell c[1], inout PointStream<Marker> output)
{
    //
    //  Unpack the cell information.
    //
    Cell cell = c[0];
    uint3 cpos = uint3((cell.id >> 24) & 0xFF,
                       (cell.id >> 16) & 0xFF,
                       (cell.id >> 8) & 0xFF);
    uint cornerMask = cell.id & 0xFF;
    uint packedCellPos = cell.id & 0xFFFFFF00;

    //
	//	The triangle table indicates which triangles to generate based
    //  on the corner mask.
    //
	for (uint i = 0; TriTable[(cornerMask * 16) + i] != -1; i += 3) {
		for (uint j = 0; j < 3; j++)
		{
			uint edge = TriTable[(cornerMask * 16) + i + j];
            
            //
            //  Discard vertices which aren't on edges 0, 8, or 3 - they will be
            //  produced by the neighboring cell.
            //
			if (edge == 0 || edge == 8 || edge == 3)
			{
				Marker m;
				m.packed = packedCellPos | edge;
				output.Append(m);
			}
		}
	}
}
