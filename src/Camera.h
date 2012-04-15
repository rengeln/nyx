///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_CAMERA_H__
#define __NYX_CAMERA_H__

#include "Frustum.h"

class Camera {
public:
    //
    //  Constructor.
    //
    Camera();

    //
    //  Destructor.
    //
    ~Camera();

    //
    //  Sets the projection matrix.
    //
    void SetProjectionMatrix(const float4x4& matrix);

    //
    //  Sets the camera's position in worldspace.
    //
    void SetPosition(const float3& pos);

    //
    //  Sets the camera's rotation in worldspace.
    //
    void SetRotation(const float4& rot);

    //
    //  Returns the camera's projection matrix.
    //
    const float4x4& GetProjectionMatrix() const;

    //
    //  Returns the camera's view matrix.
    //
    const float4x4& GetViewMatrix() const;

    //
    //  Returns the combined projection-view matrix.
    //
    const float4x4& GetCombinedMatrix() const;

    //
    //  Returns the camera's position.
    //
    const float3& GetPosition() const;

    //
    //  Returns the camera's forward vector.
    //
    const float3& GetForwardVector() const;

    //
    //  Returns the camera's up vector.
    //
    const float3& GetUpVector() const;

    //
    //  Returns the camera's right vector.
    //
    const float3& GetRightVector() const;

    //
    //  Returns the camera's frustum.
    //
    const Frustum& GetFrustum() const;

private:
    //
    //  Updates the derived members if dirty.
    //
    void Update() const;

    //
    //  Properties.
    //
    float4x4 m_projectionMatrix;
    float3 m_position;
    float4 m_rotation;
    mutable float4x4 m_viewMatrix;
    mutable float4x4 m_combinedMatrix;
    mutable float3 m_forwardVector;
    mutable float3 m_upVector;
    mutable float3 m_rightVector;
    mutable Frustum m_frustum;
    mutable bool m_dirty;
};

inline const float3& Camera::GetPosition() const
{
    return m_position;
}

#endif  // __NYX_CAMERA_H__
