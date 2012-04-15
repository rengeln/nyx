///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_FRUSTUM_H__
#define __NYX_FRUSTUM_H__

enum FrustumPlane {
    FrustumPlane_Left,
    FrustumPlane_Bottom,
    FrustumPlane_Near,
    FrustumPlane_Right,
    FrustumPlane_Top,
    FrustumPlane_Far,
    FrustumPlane_Max
};

class Frustum
{
public:
    //
    //  Default constructor.
    //
    Frustum();

    //
    //  Construct from a view-projection matrix.
    //
    Frustum(const float4x4& m);

    //
    //  Returns one of the frustum planes.
    //
    float4 GetPlane(FrustumPlane p);

    //
    //  Extract from a view-projection matrix.
    //
    Frustum& operator=(const float4x4& m);

    //
    //  Intersection test with an axis-aligned box.
    //
    bool Intersects(const box3f& box) const;

private:
    //
    //  Extract from a matrix.
    //
    void Extract(const float4x4& m);

    //
    //  Properties.
    //
    float4 m_planes[FrustumPlane_Max];
};

inline Frustum::Frustum()
{
}

inline Frustum::Frustum(const float4x4& m)
{
    Extract(m);
}

inline Frustum& Frustum::operator=(const float4x4& m)
{
    Extract(m);
    return *this;
}

inline float4 Frustum::GetPlane(FrustumPlane p)
{
    assert(p < FrustumPlane_Max);
    return m_planes[p];
}

inline bool Frustum::Intersects(const box3f& box) const
{
    for (size_t i = 0; i < FrustumPlane_Max; ++i) 
    {
        XMFLOAT3 v;
        v.x = m_planes[i].x >= 0 ? box.second.x : box.first.x;
        v.y = m_planes[i].y >= 0 ? box.second.y : box.first.y;
        v.z = m_planes[i].z >= 0 ? box.second.z : box.first.z;

        XMVECTOR _v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v)),
                 _p = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&m_planes[i]));
        if (XMVectorGetX(XMPlaneDotCoord(_p, _v)) < 0.0f)
        {
            return false;
        }
    }
    return true;
}


#endif  //  __NYX_FRUSTUM_H__
