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
	//  cornerMask: 8
	uint packed : POSITION;
};

//
//  Geometry shader for the list-cells phase.
//
//  Checks the corner mask; if all corners are negative (0) or all corners
//  are positive (0), the cell is discarded. Otherwise, it is added to the
//  cell buffer.
//
[maxvertexcount(1)]
void main(point Marker m[1], inout PointStream<Marker> output)
{
    uint mask = m[0].packed & 0xFF;
    if (mask == 0 || mask == 255)
    {
        return;
    }
    output.Append(m[0]);
}
