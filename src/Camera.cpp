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

const Frustum& Camera::GetFrustum() const
{
    Update();
    return m_frustum;
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
        m_frustum = m_combinedMatrix;

        m_dirty = false;
    }
}
