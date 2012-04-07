///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

Buffer<float3> GradientVectors;
Buffer<uint> Permutations;

//
//  3D simplex noise implementation.
//
float simplex_noise(float3 pos)
{
    const float F3 = 1.0f / 3.0f, G3 = 1.0f / 6.0f;
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;
    float n0, n1, n2, n3;
    float s = (x + y + z) * F3;
    int i = (int)floor(x + s);
    int j = (int)floor(y + s);
    int k = (int)floor(z + s);
    float t = (float)(i + j + k) * G3;
    float X0 = (float)i - t;
    float Y0 = (float)j - t;
    float Z0 = (float)k - t;
    float x0 = x - X0;
    float y0 = y - Y0;
    float z0 = z - Z0;

    int i1, j1, k1, i2, j2, k2;
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; // X Y Z order
        }
        else if (x0 >= z0) {
            i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; // X Z Y order
        }
        else { 
            i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; // Z X Y order
        } 
    } else { // x0<y0
        if (y0 < z0) { 
            i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; // Z Y X order
        }
        else if (x0 < z0) { 
            i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; // Y Z X order
        }
        else { 
            i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; // Y X Z order
        }
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.    
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = Permutations[ii+Permutations[jj+Permutations[kk]]] % 12;
    int gi1 = Permutations[ii+i1+Permutations[jj+j1+Permutations[kk+k1]]] % 12;
    int gi2 = Permutations[ii+i2+Permutations[jj+j2+Permutations[kk+k2]]] % 12;
    int gi3 = Permutations[ii+1+Permutations[jj+1+Permutations[kk+1]]] % 12;

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0) {
        n0 = 0.0;
    }
    else {
      t0 *= t0;
      n0 = t0 * t0 * (GradientVectors[gi0].x * x0 + 
                      GradientVectors[gi0].y * y0 +
                      GradientVectors[gi0].z * z0);
    }

    float t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) {
        n1 = 0.0;
    }
    else {
        t1 *= t1;
        n1 = t1 * t1 * (GradientVectors[gi1].x * x1 + 
                        GradientVectors[gi1].y * y1 +
                        GradientVectors[gi1].z * z1);
    }

    float t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) {
        n2 = 0.0;
    }
    else {
        t2 *= t2;
        n2 = t2 * t2 * (GradientVectors[gi2].x * x2 + 
                        GradientVectors[gi2].y * y2 +
                        GradientVectors[gi2].z * z2);
    }

    float t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) {
        n3 = 0.0;
    }
    else {
        t3 *= t3;
        n3 = t3 * t3 * (GradientVectors[gi3].x * x3 + 
                        GradientVectors[gi3].y * y3 +
                        GradientVectors[gi3].z * z3);
    }

    float r = 32.0f * (n0 + n1 + n2 + n3);
    return r;
}

//
//  Returns a noise value scaled to the range [a..b]
//
float simplex_noise_range(float3 xyz, float a, float b)
{
    return a + ((1.0f + simplex_noise(xyz)) / 2.0f) * (b - a);
}

