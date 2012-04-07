///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "marching_cubes.h"

//
//  Packed cell info:
//	    x : 8
//	    y : 8
//      z : 8
//      cornerMask: 8
//
struct Cell
{
    uint id : POSITION;
};

struct Index
{
	uint value : POSITION;
};

//
//  Geometry shader for the index-generation phase.
//
//  Takes as input a cell ID and outputs the indices necessary to render the triangles
//  in that cell. We want to use 16-bit indices, but stream-out can generate at a
//  minimum 32 bits of data, so we pack the indices together.
//
//  If this shader would produce an odd number of indices, it will append an extra
//  pair of indices with a value of 0 and set the high word of the last real index
//  to 0. This is interpreted during the rendering stage as a degenerate triangle
//  and ignored.
//
//  Even with the necessity of occassionally outputting 3 bytes of dummy data,
//  the memory savings produced by this optimization are substantial. In the worst
//  case (one triangles generated), 12 bytes are used, which is the same size
//  as three 32-bit indices.
//
[maxvertexcount(15)]
void main(point Cell c[1], inout PointStream<Index> output)
{
    //  These are used for the packing algorihtm
    uint counter;
    uint accum;


    //  Extract the cell position and corner mask from the packed cell ID
    Cell cell = c[0];
    uint3 cpos = uint3((cell.id >> 24) & 0xFF,
                       (cell.id >> 16) & 0xFF,
                       (cell.id >> 8) & 0xFF);
    uint cornerMask = cell.id & 0xFF;

    //  The last cell in each dimension is padding, ignore it or we'll
    //  produce bad geometry
    if (cpos.x == CellCount.x - 1 || 
        cpos.y == CellCount.y - 1 || 
        cpos.z == CellCount.z - 1)
        return;

    //  
	//	The triangle table determines how to generate triangles for the
    //  given corner mask. This is part of the normal marching cubes
    //  algorithm, just split off from the rest of it.
    //
	for (uint i = 0; TriTable[(cornerMask * 16) + i] != -1; i += 3) {
		for (uint j = 0; j < 3; j++)
		{
			uint edge = TriTable[(cornerMask * 16) + i + j];
			
			uint3 splatPos = cpos;
            splatPos += EdgeCellOffsets[edge];
            splatPos.x *= 3;
            splatPos += EdgeXOffsets[edge];

			int3 uv = int3(splatPos.x, (splatPos.z * CellCount.y) + splatPos.y, 0);
            uint index = VertexIDMap.Load(uv).x;
            accum |= (index & 0xFFFF) << (counter * 16);
            if (++counter == 2)
            {
                Index i;
                i.value = accum;
                output.Append(i);
                accum = 0;
                counter = 0;
            }
		}
	}
    //
    //  Deal with an odd number of vertices by inserting a degenerate
    //  triangle (indices 0, 0, 0)
    //
    if (counter > 0)
    {
        Index i;
        i.value = accum;
        output.Append(i);
        i.value = 0;
        output.Append(i);
    }
}
