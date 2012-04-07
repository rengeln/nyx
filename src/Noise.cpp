///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Noise.h"

const XMFLOAT3 Noise::GradiantVectors[] = 
{
    XMFLOAT3(1,1,0), XMFLOAT3(-1,1,0), XMFLOAT3(1,-1,0), XMFLOAT3(-1,-1,0),
    XMFLOAT3(1,0,1), XMFLOAT3(-1,0,1), XMFLOAT3(1,0,-1), XMFLOAT3(-1,0,-1),
    XMFLOAT3(0,1,1), XMFLOAT3(0,-1,1), XMFLOAT3(0,1,-1), XMFLOAT3(0,-1,-1)
};

const int Noise::Permutations[] = 
{
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

Noise::Noise()
{
    srand(time(0));
    int shuffledPerms[256];
    memcpy(shuffledPerms, Permutations, sizeof(Permutations));
    for (size_t i = 255; i > 0; --i)
    {
        size_t j = rand() % (i + 1);
        std::swap(shuffledPerms[j], shuffledPerms[i]);
    }
    for (uint i = 0; i < 512; i++) {
        m_permutationTable[i] = shuffledPerms[i & 255];
    }
}

Noise::~Noise()
{
}

float Noise::SampleRange(float x, float y, float z, float a, float b)
{
    float n = (1.0f + Sample(x, y, z)) * 0.5f;
    return a + (n * (b - a));
}

float Noise::Sample(float x, float y, float z) 
{
    const float F3 = 1.0f / 3.0f, G3 = 1.0f / 6.0f;
    float n0, n1, n2, n3;
    float s = (x + y + z) * F3;
    int i = (int)floor(x + s),
        j = (int)floor(y + s),
        k = (int)floor(z + s);
    float t = (float)(i + j + k) * G3;
    float X0 = (float)i - t,
          Y0 = (float)j - t,
          Z0 = (float)k - t,
          x0 = x - X0,
          y0 = y - Y0,
          z0 = z - Z0;

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
    int gi0 = m_permutationTable[ii+m_permutationTable[jj+m_permutationTable[kk]]] % 12;
    int gi1 = m_permutationTable[ii+i1+m_permutationTable[jj+j1+m_permutationTable[kk+k1]]] % 12;
    int gi2 = m_permutationTable[ii+i2+m_permutationTable[jj+j2+m_permutationTable[kk+k2]]] % 12;
    int gi3 = m_permutationTable[ii+1+m_permutationTable[jj+1+m_permutationTable[kk+1]]] % 12;

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0) {
        n0 = 0.0;
    }
    else {
      t0 *= t0;
      n0 = t0 * t0 * (GradiantVectors[gi0].x * x0 + 
                      GradiantVectors[gi0].y * y0 +
                      GradiantVectors[gi0].z * z0);
    }

    float t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) {
        n1 = 0.0;
    }
    else {
        t1 *= t1;
        n1 = t1 * t1 * (GradiantVectors[gi1].x * x1 + 
                        GradiantVectors[gi1].y * y1 +
                        GradiantVectors[gi1].z * z1);
    }

    float t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) {
        n2 = 0.0;
    }
    else {
        t2 *= t2;
        n2 = t2 * t2 * (GradiantVectors[gi2].x * x2 + 
                        GradiantVectors[gi2].y * y2 +
                        GradiantVectors[gi2].z * z2);
    }

    float t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) {
        n3 = 0.0;
    }
    else {
        t3 *= t3;
        n3 = t3 * t3 * (GradiantVectors[gi3].x * x3 + 
                        GradiantVectors[gi3].y * y3 +
                        GradiantVectors[gi3].z * z3);
    }

    float r = 32.0f * (n0 + n1 + n2 + n3);
    return r;
}
