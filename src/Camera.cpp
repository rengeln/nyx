///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"


Camera::Camera()
: m_position(float3::Zero()),
  m_rotation(float4::IdentityQuaternion()),
  m_dirty(true)
{
}

Camera::~Camera()
{
}

void Camera::SetProjectionMatrix(const float4x4& matrix)
{
    m_projectionMatrix = matrix;
    m_dirty = true;
}

void Camera::SetPosition(const float3& pos)
{
    m_position = pos;
    m_dirty = true;
}

void Camera::SetRotation(const float4& rot)
{
    m_rotation = rot;
    m_dirty = true;
}

const float4x4& Camera::GetProjectionMatrix() const
{
    return m_projectionMatrix;
}

const float4x4& Camera::GetViewMatrix() const
{
    Update();
    return m_viewMatrix;
}

const float4x4& Camera::GetCombinedMatrix() const
{
    Update();
    return m_combinedMatrix;
}

const float3& Camera::GetForwardVector() const
{
    Update();
    return m_forwardVector;
}

const float3& Camera::GetUpVector() const
{
    Update();
    return m_upVector;
}

const float3& Camera::GetRightVector() const
{
    Update();
    return m_rightVector;
}

void Camera::Update() const
{
    if (m_dirty) {
        //  Rebuild matrices.
        float4x4 rotationMatrix = float4x4::Rotation(QuaternionInverse(m_rotation));
        float4x4 transMatrix = float4x4::Translation(-m_position);
        m_viewMatrix = transMatrix * rotationMatrix;
        m_combinedMatrix = m_viewMatrix * m_projectionMatrix;

        //  Rebuild vectors.
        m_forwardVector = rotationMatrix * float3(0.0f, 0.0f, 1.0f);
        m_rightVector = rotationMatrix * float3(1.0f, 0.0f, 0.0f);
        m_upVector = rotationMatrix * float3(0.0f, 1.0f, 0.0f);

        //  Extract frustum planes.
        float4x4& m = m_combinedMatrix;
        m_frustumPlanes[FrustumPlane_Right].x = m[0][3] + m[0][0];
        m_frustumPlanes[FrustumPlane_Right].y = m[1][3] + m[1][0];
        m_frustumPlanes[FrustumPlane_Right].z = m[2][3] + m[2][0];
        m_frustumPlanes[FrustumPlane_Right].w = m[3][3] + m[3][0];
   
        m_frustumPlanes[FrustumPlane_Left].x = m[0][3] - m[0][0];
        m_frustumPlanes[FrustumPlane_Left].y = m[1][3] - m[1][0];
        m_frustumPlanes[FrustumPlane_Left].z = m[2][3] - m[2][0];
        m_frustumPlanes[FrustumPlane_Left].w = m[3][3] - m[3][0];

        m_frustumPlanes[FrustumPlane_Top].x = m[0][3] - m[0][1];
        m_frustumPlanes[FrustumPlane_Top].y = m[1][3] - m[1][1];
        m_frustumPlanes[FrustumPlane_Top].z = m[2][3] - m[2][1];
        m_frustumPlanes[FrustumPlane_Top].w = m[3][3] - m[3][1];

        m_frustumPlanes[FrustumPlane_Bottom].x = m[0][3] + m[0][1];
        m_frustumPlanes[FrustumPlane_Bottom].y = m[1][3] + m[1][1];
        m_frustumPlanes[FrustumPlane_Bottom].z = m[2][3] + m[2][1];
        m_frustumPlanes[FrustumPlane_Bottom].w = m[3][3] + m[3][1];

        m_frustumPlanes[FrustumPlane_Far].x = m[0][2];
        m_frustumPlanes[FrustumPlane_Far].y = m[1][2];
        m_frustumPlanes[FrustumPlane_Far].z = m[2][2];
        m_frustumPlanes[FrustumPlane_Far].w = m[3][2];

        m_frustumPlanes[FrustumPlane_Near].x = m[0][3] - m[0][2];
        m_frustumPlanes[FrustumPlane_Near].y = m[1][3] - m[1][2];
        m_frustumPlanes[FrustumPlane_Near].z = m[2][3] - m[2][2];
        m_frustumPlanes[FrustumPlane_Near].w = m[3][3] - m[3][2];

        m_dirty = false;
    }
}

bool Camera::Intersects(const box3f& box) const
{
    for (size_t i = 0; i < FrustumPlane_Max; ++i) 
    {
        XMFLOAT3 v;
        v.x = m_frustumPlanes[i].x >= 0 ? box.second.x : box.first.x;
        v.y = m_frustumPlanes[i].y >= 0 ? box.second.y : box.first.y;
        v.z = m_frustumPlanes[i].z >= 0 ? box.second.z : box.first.z;

        XMVECTOR _v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v)),
                 _p = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&m_frustumPlanes[i]));
        if (XMVectorGetX(XMPlaneDotCoord(_p, _v)) < 0.0f)
        {
            return false;
        }
    }
    return true;
}

