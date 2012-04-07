///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//  See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

//
//  Shader constants.
//
cbuffer Constants {
    float3 ChunkPosition;
    float3 ChunkDimensions;
    uint ChunkDepth;
	int3 CellCount;
    float3 VoxelFieldTotalSize;
    float3 VoxelFieldSubSize;
    float3 VoxelFieldOffset;
    float3 VoxelIDMapSize;
    uint4 EdgeCellOffsets[12];
    uint4 EdgeXOffsets[12];
};

Buffer<int> TriTable;
Texture3D<float> VoxelField;
Texture3D<uint2> MaterialField;
Texture2D<uint> VertexIDMap;
SamplerState VoxelSampler;

//
//  Bilinear interpolation.
//
//  Parameters:
//      [in] a, b, c, d
//          Corner values, starting from top-left and going clockwise.
//      [in] s, t
//          Blending factors in the X and Y axes.
//
float4 Bilerp(float4 a, float4 b, float4 c, float4 d, float s, float t)
{
    float4 _1 = (a * (1 - s) + b * s) * (1 - t);
    float4 _2 = (d * (1 - s) + c * s) * t;
    float4 _3 = _1 + _2;
    return _3;
}

//
//  Trilinear interpolation.
//
//  Parameters:
//      [in] a, b, c, d, e, f, g, h
//          Corner values, starting from near-top-left and going clockwise,
//          then far-top-left and going clockwise again.
//      [in] s, t, u
//          Blending factors in the X, Y and Z axes.
//
float4 Trilerp(float4 a, float4 b, float4 c, float4 d,
              float4 e, float4 f, float4 g, float4 h,
              float s, float t, float u)
{
    float4 _1 = Bilerp(a, b, c, d, s, t);
    float4 _2 = Bilerp(e, f, g, h, s, t);
    float4 _3 = lerp(_1, _2, u);
    return _3;
}

//
//  Returns the density of the voxel field at the given voxel-space coordinates.
//
float Density(float3 v)
{
	v /= CellCount - 1;
	v *= VoxelFieldSubSize / VoxelFieldTotalSize;
	v += VoxelFieldOffset / VoxelFieldTotalSize;
	return VoxelField.SampleLevel(VoxelSampler, v, 0).r;
}

//
//  Returns the material weights of the voxel field at the given voxel-space coordinates.
//
//  Up to eight material weights are packed into 8-bit components.
//
uint2 Material(float3 f)
{
    f -= 0.5f;      // Account for the fact that nearest-neighbor sampling does not
                    // use a half-pixel offset, whereas SampleLevel() does.
    f /= CellCount - 1;
    f *= VoxelFieldSubSize / VoxelFieldTotalSize;
    f += VoxelFieldOffset / VoxelFieldTotalSize;
    
    uint4 v = uint4(uint3(f * VoxelFieldTotalSize), 0);
    
    //  Retrive the material weights from the voxel field using nearest-neighbor sampling
    uint2 packed[8] =
    {
        MaterialField.Load(v),
        MaterialField.Load(v + uint4(1, 0, 0, 0)),
        MaterialField.Load(v + uint4(1, 1, 0, 0)),
        MaterialField.Load(v + uint4(0, 1, 0, 0)),
        MaterialField.Load(v + uint4(0, 0, 1, 0)),
        MaterialField.Load(v + uint4(1, 0, 1, 0)),
        MaterialField.Load(v + uint4(1, 1, 1, 0)),
        MaterialField.Load(v + uint4(0, 1, 1, 0))
    };

    //  Unpack the weights into floats so they can be filtered
    float4 weights1[8], weights2[8];
    [unroll] for (uint i = 0; i < 8; i++)
    {
        weights1[i] = float4(float((packed[i].x >> 24) & 0xFF) / 255.0f,
                             float((packed[i].x >> 16) & 0xFF) / 255.0f,
                             float((packed[i].x >> 8) & 0xFF) / 255.0f,
                             float((packed[i].x >> 0) & 0xFF) / 255.0f);
        weights2[i] = float4(float((packed[i].y >> 24) & 0xFF) / 255.0f,
                             float((packed[i].y >> 16) & 0xFF) / 255.0f,
                             float((packed[i].y >> 8) & 0xFF) / 255.0f,
                             float((packed[i].y >> 0) & 0xFF) / 255.0f);
    }

    //  Calculate the interpolation weights
    float3 mu = float3(f - floor(f));

    //  Filter the results
    float4 r1 = Trilerp(weights1[0], weights1[1], weights1[2], weights1[3],
                        weights1[4], weights1[5], weights1[6], weights1[7],
                        mu.x, mu.y, mu.z);
    float4 r2 = Trilerp(weights2[0], weights2[1], weights2[2], weights2[3],
                        weights2[4], weights2[5], weights2[6], weights2[7],
                        mu.x, mu.y, mu.z);

    //  Pack the results
    uint2 r;
    r.x =  (uint(r1.x * 255.0f) << 24) +
           (uint(r1.y * 255.0f) << 16) +
           (uint(r1.z * 255.0f) << 8) +
           (uint(r1.w * 255.0f) << 0);
    r.y =  (uint(r2.x * 255.0f) << 24) +
           (uint(r2.y * 255.0f) << 16) +
           (uint(r2.z * 255.0f) << 8) +
           (uint(r2.w * 255.0f) << 0);

    return r;
}
