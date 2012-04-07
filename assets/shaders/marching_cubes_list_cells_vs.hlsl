///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "marching_cubes.h"

struct Cell
{
	uint id : POSITION;
};

struct Marker
{
	//	x : 8
	//	y : 8
	//  z : 8
	//  cornerMask : 8
	uint packed : POSITION;
};

//
//  Vertex shader for the list cells phase.
//
Marker main(Cell cell)
{
    //  Determine the cell's position in voxel space given its id number
	uint n = cell.id;
	uint w = CellCount.x, h = CellCount.y, d = CellCount.z;
	uint3 cpos;
	cpos.x = n % w;
	cpos.y = (n % (w * h)) / w;
	cpos.z = n / (w * h); 

	//	Calculate the bounding box for the current cell.
	//	The input point is the near bottom left corner.
	float3 cellMin = float3(cpos.x, cpos.y, cpos.z);
	float3 cellMax = float3(cpos.x + 1, cpos.y + 1, cpos.z + 1);
	
	//	Generate corner vertices.
	float3 corners[8];
	corners[0] = float3(cellMin.x, cellMin.y, cellMin.z);
	corners[1] = float3(cellMax.x, cellMin.y, cellMin.z);
	corners[2] = float3(cellMax.x, cellMin.y, cellMax.z);
	corners[3] = float3(cellMin.x, cellMin.y, cellMax.z);
	corners[4] = float3(cellMin.x, cellMax.y, cellMin.z);
	corners[5] = float3(cellMax.x, cellMax.y, cellMin.z);
	corners[6] = float3(cellMax.x, cellMax.y, cellMax.z);
	corners[7] = float3(cellMin.x, cellMax.y, cellMax.z);

	//  Sample each corner, store the samples and mark a bitmask according to which corners
	//  return negative densities. The bitmask is then used as an index into a lookup table
	//  telling us which edges should produce vertices.
	float cornerSamples[8];
	int cornerMask = 0;
	for (int i = 0; i < 8; i++) {
		cornerSamples[i] = Density(corners[i]);
		if (cornerSamples[i] > 0.0f) {
			cornerMask |= (1 << i);
		}
	}

    //  Pack the cell position and corner mask into a single 32-bit value
	uint packedCellPos = ((cpos.x & 0xFF) << 24) |
	    				 ((cpos.y & 0xFF) << 16) |
						 ((cpos.z & 0xFF) << 8);
    Marker m;
    m.packed = packedCellPos | cornerMask;
    return m;
}
