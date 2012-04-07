///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_VECTOR_H__
#define __NYX_VECTOR_H__

inline float Lerp(float a, float b, float t)
{
    return a + ((b - a) * t);
}

template <typename T>
inline bool Equals(const T& a, const T& b)
{
    return a == b;
}

inline bool Equals(float a, float b)
{
    return std::fabs(a - b) < 0.0001f;
}

inline bool Equals(double a, double b)
{
    return std::fabs(a - b) < 0.0001f;
}

template <typename T, size_t N>
class VectorBase
{
public:
};

template <typename T>
class VectorBase<T, 2>
{
public:
    T x, y;
};

template <typename T>
class VectorBase<T, 3>
{
public:
    T x, y, z;
};

template <typename T>
class VectorBase<T, 4>
{
public:
    T x, y, z, w;
};

template <typename T, size_t N>
class Vector : public VectorBase<T, N>
{
    static_assert(N >= 2 && N <= 4, "N must be between 2 and 4");

public:
    //
    //  Default constructor.
    //
    Vector();

    //
    //  Construct from an arbitrary vector.
    //
    template <typename U, size_t M>
    Vector(const Vector<U, M>& cpy);
    
    //
    //  Construct from an arbitrary vector with one additional component value.
    //
    template <typename U, size_t M>
    Vector(const Vector<U, M>& cpy, const T& a);

    //
    //  Construct from an arbitrary vector with two additional component values.
    //
    template <typename U, size_t M>
    Vector(const Vector<U, M>& cpy, const T& a, const T& b);

    //
    //  Construct from component values.
    //
    Vector(const T& a, const T& b);
    Vector(const T& a, const T& b, const T& c);
    Vector(const T& a, const T& b, const T& c, const T& d);

    //
    //  Assignment from an arbitrary vector.
    //
    template <typename U, size_t M>
    Vector<T, N>& operator=(const Vector<U, M>& rhs);

    //
    //  Memberwise arithmatic with assignment operators.
    //
    Vector<T, N>& operator+=(const Vector<T, N>& rhs);
    Vector<T, N>& operator-=(const Vector<T, N>& rhs);
    Vector<T, N>& operator*=(const Vector<T, N>& rhs);
    Vector<T, N>& operator/=(const Vector<T, N>& rhs);

    //
    //  Scalar arithmetic with assignment operators.
    //
    Vector<T, N>& operator+=(const T& rhs);
    Vector<T, N>& operator-=(const T& rhs);
    Vector<T, N>& operator*=(const T& rhs);
    Vector<T, N>& operator/=(const T& rhs);

    //
    //  Array index operator.
    //
    T& operator[](size_t n);
    T operator[](size_t n) const;

    //
    //  Length and squared length operators.
    //
    friend T Length(const Vector<T, N>& lhs);
    friend T Length2(const Vector<T, N>& lhs);

    //
    //  Comparison operator.
    //
    template <typename U, size_t O>
    friend bool operator==(const Vector<U, O>& lhs, const Vector<U, O>& rhs);

    //
    //  Negation operator.
    //
    template <typename U, size_t O>
    friend Vector<U, O> operator-(const Vector<U, O>& lhs);

    //
    //  Memberwise arithmatic operators.
    //
    template <typename U, size_t O>
    friend Vector<U, O> operator+(const Vector<U, O>& lhs, const Vector<U, O>& rhs);
    template <typename U, size_t O>
    friend Vector<U, O> operator-(const Vector<U, O>& lhs, const Vector<U, O>& rhs);
    template <typename U, size_t O>
    friend Vector<U, O> operator*(const Vector<U, O>& lhs, const Vector<U, O>& rhs);
    template <typename U, size_t O>
    friend Vector<U, O> operator/(const Vector<U, O>& lhs, const Vector<U, O>& rhs);
    

    //
    //  Scalar arithmetic operators.
    //
    template <typename U, size_t O>
    friend Vector<U, O> operator+(const Vector<U, O>& lhs, const U& rhs);
    template <typename U, size_t O>
    friend Vector<U, O> operator-(const Vector<U, O>& lhs, const U& rhs);
    template <typename U, size_t O>
    friend Vector<U, O> operator*(const Vector<U, O>& lhs, const U& rhs);
    template <typename U, size_t O>
    friend Vector<U, O> operator/(const Vector<U, O>& lhs, const U& rhs);

    //
    //  Dot product.
    //
    template <typename U, size_t O>
    friend U Dot(const Vector<U, O>& lhs, const Vector<U, O>& rhs);

    //
    //  Cross product (3D vectors only)
    //
    template <typename U, size_t O>
    friend Vector<U, 3> Cross(const Vector<U, 3>& lhs, const Vector<U, 3>& rhs);

    //
    //  Linear interpolation between vectors.
    //
    template <typename U, size_t O, typename V>
    friend Vector<U, O> Lerp(const Vector<U, O>& lhs, const Vector<U, O>& rhs, const V& t);

    //
    //  Returns the normalized form of a vector.
    //
    template <typename U, size_t O>
    friend Vector<U, O> Normalize(const Vector<U, O>& lhs);

    //
    //  Returns the result of concatenating two quaternions.
    //
    template <typename U>
    friend Vector<U, 4> QuaternionConcatenate(const Vector<U, 4>& lhs, const Vector<U, 4>& rhs);

    //
    //  Returns the result of slerping between two quaternions.
    //
    template <typename U>
    friend Vector<U, 4> QuaternionSlerp(const Vector<U, 4>& lhs, const Vector<U, 4>& rhs, const U& t);

    //
    //  Returns the inverse of a quaternion.
    //
    template <typename U>
    friend Vector<U, 4> QuaternionInverse(const Vector<U, 4>& lhs);

    //
    //  Returns a zero vector.
    //
    static Vector<T, N> Zero();

    //
    //  Returns a vector with a value replicated in every component.
    //
    static Vector<T, N> Replicate(const T& v);

    //
    //  Returns an identity quaternion.
    //
    static Vector<T, N> IdentityQuaternion();

    //
    //  Returns a quaternion based on roll, pitch, and yaw angles.
    //
    static Vector<T, N> RotationQuaternion(const T& roll, const T& pitch, const T& yaw);

    //
    //  Returns a quaternion based on an arbitrary axis of rotation.
    //
    static Vector<T, N> RotationQuaternion(const Vector<T, 3>& axis, const T& angle);
};

template <typename T, size_t N>
inline Vector<T, N>::Vector()
{
}

template <typename T, size_t N> template <typename U, size_t M>
inline Vector<T, N>::Vector(const Vector<U, M>& cpy)
{
    *this = cpy;
}

template <typename T, size_t N> template <typename U, size_t M>
inline Vector<T, N>::Vector(const Vector<U, M>& cpy, const T& a)
{
    static_assert(N == M + 1, "Requires N == M + 1");
    *this = cpy;
    (*this)[N - 1] = a;
}

template <typename T, size_t N> template <typename U, size_t M>
inline Vector<T, N>::Vector(const Vector<U, M>& cpy, const T& a, const T& b)
{
    static_assert(N == M + 2, "Requires N == M + 2");
    *this = cpy;
    (*this)[N - 2] = a;
    (*this)[N - 1] = b;
}

template <typename T, size_t N>
inline Vector<T, N>::Vector(const T& a, const T& b)
{
    static_assert(N == 2, "Requires N == 2");
    x = a;
    y = b;
}

template <typename T, size_t N>
inline Vector<T, N>::Vector(const T& a, const T& b, const T& c)
{
    static_assert(N == 3, "Requires N == 3");
    x = a;
    y = b;
    z = c;
}

template <typename T, size_t N>
inline Vector<T, N>::Vector(const T& a, const T& b, const T& c, const T& d)
{
    static_assert(N == 4, "Requires N == 4");
    x = a;
    y = b;
    z = c;
    w = d;
}

template <typename T, size_t N> template <typename U, size_t M>
inline Vector<T, N>& Vector<T, N>::operator=(const Vector<U, M>& rhs)
{
    for (size_t i = 0; i < min(N, M); i++)
    {
        (*this)[i] = static_cast<T>(rhs[i]);
    }
    if (N > M)
    {
        for (size_t i = M; i < N; i++)
        {
            (*this)[i] = static_cast<T>(0);
        }
    }
    return *this;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator+=(const Vector<T, N>& rhs)
{
    return *this = *this + rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator-=(const Vector<T, N>& rhs)
{
    return *this = *this - rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator*=(const Vector<T, N>& rhs)
{
    return *this = *this * rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator/=(const Vector<T, N>& rhs)
{
    return *this = *this / rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator+=(const T& rhs)
{
    return *this = *this + rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator-=(const T& rhs)
{
    return *this = *this - rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator*=(const T& rhs)
{
    return *this = *this * rhs;
}

template <typename T, size_t N>
inline Vector<T, N>& Vector<T, N>::operator/=(const T& rhs)
{
    return *this = *this / rhs;
}

template <typename T, size_t N>
inline T& Vector<T, N>::operator[](size_t i)
{
    assert(i < N);
    return reinterpret_cast<T*>(this)[i];
}

template <typename T, size_t N>
inline T Vector<T, N>::operator[](size_t i) const
{
    assert(i < N);
    return reinterpret_cast<const T*>(this)[i];
}

template <typename T, size_t N>
inline T Length(const Vector<T, N>& lhs)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    return sqrt(Length2(lhs));
}

template <typename T, size_t N>
inline T Length2(const Vector<T, N>& lhs)
{
    return Dot(lhs, lhs);
}

template <typename T, size_t N>
inline bool operator==(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    for (size_t i = 0; i < N; i++)
    {
        if (!Equals(lhs[i], rhs[i]))
        {
            return false;
        }
    }
    return true;
}

template <typename T, size_t N>
inline Vector<T, N> operator-(const Vector<T, N>& lhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = -lhs[i];
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator+(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] + rhs[i];
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator-(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator*(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] * rhs[i];
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator/(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] / rhs[i];
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator+(const Vector<T, N>& lhs, const T& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] + rhs;
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator-(const Vector<T, N>& lhs, const T& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] - rhs;
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator*(const Vector<T, N>& lhs, const T& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] * rhs;
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> operator/(const Vector<T, N>& lhs, const T& rhs)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] / rhs;
    }
    return result;
}

template <typename T, size_t N>
inline T Dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    T result = 0;
    for (size_t i = 0; i < N; i++)
    {
        result += lhs[i] * rhs[i];
    }
    return result;
}

template <typename T>
inline Vector<T, 3> Cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs)
{
    return Vector<T, 3>(lhs.y * rhs.z - lhs.z * rhs.y,
                        lhs.z * rhs.x - lhs.x * rhs.z,
                        lhs.x * rhs.y - lhs.y * rhs.x);
}

template <typename T, size_t N, typename U>
inline Vector<T, N> Lerp(const Vector<T, N>& lhs, const Vector<T, N>& rhs, const U& t)
{
    static_assert(boost::is_float<U>::value, "Requires is_float<U>");

    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = lhs[i] + static_cast<T>(static_cast<U>(rhs[i] - lhs[i]) * t); 
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> Normalize(const Vector<T, N>& lhs)
{
    return lhs / lhs.Length();
}

template <typename T, size_t N>
inline Vector<T, N> Vector<T, N>::Zero()
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = 0;
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> Vector<T, N>::Replicate(const T& v)
{
    Vector<T, N> result;
    for (size_t i = 0; i < N; i++)
    {
        result[i] = v;
    }
    return result;
}

template <typename T, size_t N>
inline Vector<T, N> Vector<T, N>::IdentityQuaternion()
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    static_assert(N == 4, "Requires N == 4");
    return Vector<T, N>(static_cast<T>(0),
                        static_cast<T>(0),
                        static_cast<T>(0),
                        static_cast<T>(1));
}

template <typename T, size_t N>
inline Vector<T, N> Vector<T, N>::RotationQuaternion(const T& pitch, const T& yaw, const T& roll)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    static_assert(N == 4, "Requires N == 4");
    Vector<float, N> result;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result),
                  XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
    return Vector<T, N>(result);
}

template <typename T, size_t N>
inline Vector<T, N> Vector<T, N>::RotationQuaternion(const Vector<T, 3>& axis, const T& angle)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    static_assert(N == 4, "Requires N == 4");
    Vector<float, N> result;
    Vector<float, 3> xyz(axis);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(result),
                  XMQuaternionRotationAxis(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&xyz)),
                                           angle));
    return Vector<T, N>(result);
}

template <typename T>
Vector<T, 4> QuaternionConcatenate(const Vector<T, 4>& lhs, const Vector<T, 4>& rhs)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    Vector<float, 4> result,
                     a(lhs),
                     b(rhs);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(result),
                  XMQuaternionMultiply(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&a)),
                                       XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&b))));
    return Vector<T, 4>(result);
}

template <typename T>
Vector<T, 4> QuaternionSlerp(const Vector<T, 4>& lhs, const Vector<T, 4>& rhs, const T& t)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    Vector<float, 4> result,
                     a(lhs),
                     b(rhs);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(result),
                  XMQuaternionSlerp(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&a)),
                                    XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&b)),
                                    static_cast<T>(t)));
    return Vector<T, 4>(result);
}

template <typename T>
Vector<T, 4> QuaternionInverse(const Vector<T, 4>& lhs)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    Vector<float, 4> result, 
                     a(lhs);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result),
                  XMQuaternionInverse(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&a))));
    return Vector<T, 4>(result);
}

typedef Vector<float, 2> float2;
typedef Vector<float, 3> float3;
typedef Vector<float, 4> float4;
typedef Vector<double, 2> double2;
typedef Vector<double, 3> double3;
typedef Vector<double, 4> double4;
typedef Vector<int, 2> int2;
typedef Vector<int, 3> int3;
typedef Vector<int, 4> int4;
typedef Vector<unsigned int, 2> uint2;
typedef Vector<unsigned int, 3> uint3;
typedef Vector<unsigned int, 4> uint4;
typedef Vector<char, 2> byte2;
typedef Vector<char, 3> byte3;
typedef Vector<char, 4> byte4;
typedef Vector<unsigned char, 2> ubyte2;
typedef Vector<unsigned char, 3> ubyte3;
typedef Vector<unsigned char, 4> ubyte4;
typedef Vector<short, 2> short2;
typedef Vector<short, 3> short3;
typedef Vector<short, 4> short4;
typedef Vector<unsigned short, 2> ushort2;
typedef Vector<unsigned short, 3> ushort3;
typedef Vector<unsigned short, 4> ushort4;

typedef std::pair<float2, float2> box2f;
typedef std::pair<float3, float3> box3f;
typedef std::pair<float4, float4> box4f;
typedef std::pair<double2, double2> box2d;
typedef std::pair<double3, double3> box3d;
typedef std::pair<double4, double4> box4d;
typedef std::pair<int2, int2> box2i;
typedef std::pair<int3, int3> box3i;
typedef std::pair<int4, int4> box4i;
typedef std::pair<uint2, uint2> box2u;
typedef std::pair<uint3, uint3> box3u;
typedef std::pair<uint4, uint4> box4u;

#endif  // __NYX_VECTOR_H__
