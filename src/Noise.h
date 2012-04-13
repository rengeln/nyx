///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_NOISE_H__
#define __NYX_NOISE_H__

//
//  Simplex noise implementation.
//
//  Based on http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
//
class Noise : public boost::noncopyable {
public:
    //
    //  Constructor.
    //
    Noise();

    //
    //  Destructor.
    //
    ~Noise();

    //
    //  Samples noise in 3D space.
    //
    float Sample(float x, float y, float z);

    //
    //  Samples noise in 3D space and normalizes the result to a given range.
    //
    float SampleRange(float x, float y, float z, float a, float b);

    //
    //  Fills a 3D grid with noise.
    //
    //  Parameters:
    //      [in] ptr
    //          Pointer to the destination grid.
    //      [in] x, y, z
    //          Grid dimensions.
    //      [in] xf, yf, zf
    //          Frequency to sample in each direction.
    //
    void Fill(float* ptr, size_t x, size_t y, size_t z, float xf, float yf, float zf);

private:
    //
    //  Properties.
    //
    static const XMFLOAT3 GradiantVectors[];
    static const int Permutations[];

    int m_permutationTable[512];
};

#endif  // __NYX_NOISE_H__
