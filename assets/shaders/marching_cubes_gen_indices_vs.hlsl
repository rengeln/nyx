///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

//
//	Passthrough vertex shader for the index-generation phase.
//
uint main(uint n : POSITION) : POSITION
{
	return n;
}

