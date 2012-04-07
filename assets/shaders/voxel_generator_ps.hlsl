///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "simplex_noise.h"
#include "marching_cubes.h" 

struct Input
{
    float4 pos : SV_Position;
    float3 cellpos : TEXCOORD0;
};

struct Output
{
    float density : SV_Target0;
    uint2 material : SV_Target1;
};

//
//  Pixel shader for the voxel generator.
//
Output main(Input input)
{
    Output output;

    //  Calculate worldspace positions
    float3 cellpos = (input.cellpos / VoxelFieldSubSize);
    cellpos *= ChunkDimensions;
    float3 pos = ChunkPosition + cellpos;
    float3 pos2D = float3(pos.x, 0.0f, pos.z);

    //  Simple terrain generation algorithm using multiple noise octaves
    float3 mtn_warp = float3(simplex_noise(pos / 313.758),
                             0,
                             simplex_noise(pos / 353.921));
    float3 mtn_pos = pos2D + mtn_warp * 64.0f;
    float mntn = simplex_noise_range(mtn_pos / 4731.1f, -0.2f, 1.0f);
    float elev = simplex_noise_range(mtn_pos / 1213.0f, 0.0f, 1.0f);
    float disp = simplex_noise_range(mtn_pos / 637.0f, 0.0f, 1.0f);
    float height = 200.0f + ((mntn * (elev * elev)) * 1200.0f) + ((disp * disp) * 200.0f);

    float rough = min(simplex_noise_range(pos / 327.0f, 0.1f, 1.0f), elev);
    float dirt = rough;
    rough = pow(rough, 2.3f);

    float3 warp = float3(simplex_noise(pos / 27.758),
                         simplex_noise(pos / 29.572),
                         simplex_noise(pos / 31.921));
    float3 warp_pos = pos + warp * 8.0f;

    float surface_density = -(pos.y - height);
    float cave_density = ((1.0f + simplex_noise(warp_pos / 97.0f)) / 2.0f) * -96.0f;
    float density = surface_density + (cave_density * rough);
    float detail = ((1.0f + simplex_noise(warp_pos / 4.0f)) / 2.0f) * -2.0f;

    density += (detail * rough);

    //density -= (float)ChunkDepth * 0.1f;
                                //  Use this for a minecraft style world
    output.density = density; //sign(density);   //  sign(density);

    //  Calculate the material
    float grass = 1.0f - rough;
    float light_factor = simplex_noise_range(pos / 63.0f, 0.0f, 1.0f);
    float4 mat1 = float4(0, grass * light_factor, grass - (grass * light_factor), dirt);
    float4 mat2 = float4(0, 0, 0, 0);
    
    output.material.x = (uint(mat1.x * 255.0f) << 24) |
                        (uint(mat1.y * 255.0f) << 16) |
                        (uint(mat1.z * 255.0f) << 8) |
                        (uint(mat1.w * 255.0f) << 0);
    output.material.y = (uint(mat2.x * 255.0f) << 24) |
                        (uint(mat2.y * 255.0f) << 16) |
                        (uint(mat2.z * 255.0f) << 8) |
                        (uint(mat2.w * 255.0f) << 0); 

    return output;
}
