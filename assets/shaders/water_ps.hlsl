///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

struct PS_Out {
	float4 color : SV_Target;
};

struct PS_In {
	float4 position : SV_Position;
};

//
//  Pass-through pixel shader for rendering lines.
//
PS_Out main(PS_In input)
{
    PS_Out output;
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return output;
}
