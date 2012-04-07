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
//	    edge : 8
//
struct Marker
{
	uint packed : POSITION;
};

struct Vertex
{
	float3 position : POSITION;
	uint normal : NORMAL;
    uint2 material : TEXCOORD0;
};

//
//  Vertex shader for the vertex-generation phase.
//
//  Each cell can emit up to three vertices on edges 0, 3, and 8. It's actually
//  more efficient to calculate all three vertices three different times than to
//  branch and only calculate the vertex for the edge packed into the marker.
//
Vertex main(Marker marker)
{
    //
    //  Normals are calculated by taking the partial derivative of the
    //  density function in each direction; this is calculated by slightly
    //  offsetting the position of the vertex.
    //
    //  The quality of the normals vary wildly depending on the offset
    //  value; after some experimentation I've arrived at this function
    //  which works nicely.
    //
	float3 normalOffset = (CellCount / VoxelFieldTotalSize) * 0.25;

	//
	//	Unpack the cell positions and edge index.
	//
	uint3 cellPos;
	cellPos.x = (marker.packed >> 24) & 0xFF;
	cellPos.y = (marker.packed >> 16) & 0xFF;
	cellPos.z = (marker.packed >> 8) & 0xFF;
    uint edge = marker.packed & 0xFF;

    //
	//	Calculate the bounding box for the current cell.
	//	The input point is the near bottom left corner.
    //
	float3 cellMin = float3(cellPos.x, cellPos.y, cellPos.z);
	float3 cellMax = float3(cellPos.x + 1, cellPos.y + 1, cellPos.z + 1);
	
    //
	//	Store the relevant corners (0, 1, 3 and 4)
    //
	float3 corners[5];
	corners[0] = float3(cellMin.x, cellMin.y, cellMin.z);
	corners[1] = float3(cellMax.x, cellMin.y, cellMin.z);
	corners[3] = float3(cellMin.x, cellMin.y, cellMax.z);
	corners[4] = float3(cellMin.x, cellMax.y, cellMin.z);

    //
	//  Sample each relevant corner and store the results.
	//
    float cornerSamples[5];
    cornerSamples[0] = Density(corners[0]);
    cornerSamples[1] = Density(corners[1]);
    cornerSamples[3] = Density(corners[3]);
    cornerSamples[4] = Density(corners[4]);

    //
	//	Generate vertices along all possible edges.
    //
	float3 edgeVertices[9];
	edgeVertices[0] = lerp(corners[0], corners[1],
		-cornerSamples[0] / (cornerSamples[1] - cornerSamples[0]));
	edgeVertices[3] = lerp(corners[3], corners[0],
		-cornerSamples[3] / (cornerSamples[0] - cornerSamples[3]));
	edgeVertices[8] = lerp(corners[0], corners[4],
		-cornerSamples[0] / (cornerSamples[4] - cornerSamples[0]));

    //
    //  Extract the position and density for this vertex.
    //
	float3 pos = edgeVertices[edge];
    float d = Density(pos);

    //
    //  Calculate normals and pack them into a single 32-bit value.
    //
	float dx = Density(float3(pos.x + normalOffset.x, pos.y, pos.z)),
		  dy = Density(float3(pos.x, pos.y + normalOffset.y, pos.z)),
		  dz = Density(float3(pos.x, pos.y, pos.z + normalOffset.z));	
	float3 normal = normalize(float3(dx - d, dy - d, dz - d));
	uint3 inormal = uint3((normal * 128.0f) + 127.0f) & 0xFF;
	uint packedNormal = (inormal.x << 24) + (inormal.y << 16) + inormal.z;

    //
    //  Calculate the material.
    //
    uint2 material = Material(pos);

    //
    //  Scale the vertex coordinates to world space.
    //
	pos *= (ChunkDimensions / (CellCount - 1));

    //
    //  Output the vertex.
    //
	Vertex v;
	v.position = pos;
	v.normal = packedNormal;
    v.material = material;
	return v;
}
