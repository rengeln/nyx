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

float smooth_snap(float t, float m)
{
  // input: t in [0..1]
  // maps input to an output that goes from 0..1,
  // but spends most of its time at 0 or 1, except for
  // a quick, smooth jump from 0 to 1 around input values of 0.5.
  // the slope of the jump is roughly determined by 'm'.
  // note: 'm' shouldn't go over ~16 or so (precision breaks down).

  //float t1 =     pow((  t)*2, m)*0.5;
  //float t2 = 1 - pow((1-t)*2, m)*0.5;
  //return (t > 0.5) ? t2 : t1;
  
  // optimized:
  float c = (t > 0.5) ? 1 : 0;
  float s = 1-c*2;
  return c + s*pow((c+s*t)*2, m)*0.5;  
}

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

    float3 warp = float3(simplex_noise(pos / 27.758),
                         simplex_noise(pos / 29.572),
                         simplex_noise(pos / 31.921));
    float3 warpPos = pos + warp * 8.0f;
    float3 warpPos2D = float3(warpPos.x, 0, warpPos.z);

    float ocean = smooth_snap(simplex_noise_range(pos2D / 41731.0f, 0, 0.8f), 8.0f);

    float mountainTerm1 = (pos.x / 4000.0f) + (pos.z / 4000.0f) + simplex_noise(pos2D / 6000.0f) * 3.0f;
    float mountain1 = pow((1.0f + sin(mountainTerm1)) / 2.0f, 5.0f) * (1.0f - ocean);

    float mountainTerm2 = (pos.x / 5000.0f) - (pos.z / 5000.0f) + simplex_noise(pos2D / 8000.0f) * 3.0f;
    float mountain2 = pow((1.0f + sin(mountainTerm2)) / 2.0f, 5.0f) * (1.0f - ocean);

    float valleyTerm = (pos.x / 2000.0f) - (pos.z / 2000.0f) + simplex_noise(pos2D / 4000.0f) * 5.0f;
    float valley = 1.0f - pow((1.0f + sin(valleyTerm)) / 2.0f, 2.0f);

    float riverTerm = (pos.x / 3000.0f) + (pos.z / 3000.0f) + simplex_noise(pos2D / 5000.0f) * 5.0f;
    float river = pow((1.0f + sin(riverTerm)) / 2.0f, 14.0f);

    float featureTermX = (pos.x / 500.0f) + simplex_noise(pos / 1777.0f) * 2.0f;
    float featureTermZ = (pos.z / 750.0f) + simplex_noise(pos / 1533.0f) * 2.0f;
    float featurePower = pow(simplex_noise_range(pos2D / 2317.0f, 0.0f, 1.0f), 1.5f);
    float feature = smooth_snap(((2.0f + ((sin(featureTermX) + sin(featureTermZ)))) / 4.0f), 12) * featurePower;

    float height = 100.0f + (ocean * -1000.0f) + (mountain1 * valley * 1000.0f) + (mountain2 * valley * 1000.0f) + (river * -200.0f) + (feature * 150.0f);
    output.density = -(pos.y - height);

    float detailTerm1 = simplex_noise_range(pos / 531.1f, 0.0f, 1.0f);
    float detailPower1 = simplex_noise_range(pos / 1173.0f, 0.0f, 1.0f) * saturate((mountain1 + mountain2) * 2.0f);
    float detail1 = detailTerm1 * detailPower1;

    float detailTerm2 = simplex_noise(warpPos / 131.0f);
    float detailPower2 = simplex_noise_range(pos / 397.0f, 0.25f, 1.0f) * saturate(mountain1 + mountain2);
    float detail2 = detailTerm2 * detailPower2;

    float detailTerm3 = simplex_noise(warpPos / 21.0f);
    float detailPower3 = simplex_noise_range(pos / 97.0f, 0.5f, 1.0f) * saturate(mountain1 + mountain2);
    float detail3 = detailTerm3 * detailPower3;

    output.density += (detail1 * -400.0f) + (detail2 * 50.0f) + (detail3 * 2.0f);
    //output.density = sign(output.density);

    //  Calculate the material
    float rock = saturate((mountain1 + mountain2) * 2.0f);
    float sand = (1.0f - rock) * saturate(ocean * 12.0f);
    float dirt = ((1.0f - rock) - sand) * saturate(river * 2.0f);
    float grass = ((1.0f - rock) - sand) - dirt;

    float lightFactor = (1.0f + sin((pos.x / 733.0f) + (pos.z / 905.0f) + simplex_noise(pos / 1313.0f) * 2.0f)) / 2.0f;
    float4 mat1 = float4(dirt, grass * lightFactor, grass - (grass * lightFactor), rock * lightFactor);
    float4 mat2 = float4(rock - (rock * lightFactor), sand, 0, 0);
    
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
