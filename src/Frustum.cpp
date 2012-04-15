///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Frustum.h"

void Frustum::Extract(const float4x4& m)
{
    m_planes[FrustumPlane_Right].x = m[0][3] + m[0][0];
    m_planes[FrustumPlane_Right].y = m[1][3] + m[1][0];
    m_planes[FrustumPlane_Right].z = m[2][3] + m[2][0];
    m_planes[FrustumPlane_Right].w = m[3][3] + m[3][0];
   
    m_planes[FrustumPlane_Left].x = m[0][3] - m[0][0];
    m_planes[FrustumPlane_Left].y = m[1][3] - m[1][0];
    m_planes[FrustumPlane_Left].z = m[2][3] - m[2][0];
    m_planes[FrustumPlane_Left].w = m[3][3] - m[3][0];

    m_planes[FrustumPlane_Top].x = m[0][3] - m[0][1];
    m_planes[FrustumPlane_Top].y = m[1][3] - m[1][1];
    m_planes[FrustumPlane_Top].z = m[2][3] - m[2][1];
    m_planes[FrustumPlane_Top].w = m[3][3] - m[3][1];

    m_planes[FrustumPlane_Bottom].x = m[0][3] + m[0][1];
    m_planes[FrustumPlane_Bottom].y = m[1][3] + m[1][1];
    m_planes[FrustumPlane_Bottom].z = m[2][3] + m[2][1];
    m_planes[FrustumPlane_Bottom].w = m[3][3] + m[3][1];

    m_planes[FrustumPlane_Far].x = m[0][2];
    m_planes[FrustumPlane_Far].y = m[1][2];
    m_planes[FrustumPlane_Far].z = m[2][2];
    m_planes[FrustumPlane_Far].w = m[3][2];

    m_planes[FrustumPlane_Near].x = m[0][3] - m[0][2];
    m_planes[FrustumPlane_Near].y = m[1][3] - m[1][2];
    m_planes[FrustumPlane_Near].z = m[2][3] - m[2][2];
    m_planes[FrustumPlane_Near].w = m[3][3] - m[3][2];
}
