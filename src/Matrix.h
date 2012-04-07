///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_MATRIX_H__
#define __NYX_MATRIX_H__

template <typename T, size_t M, size_t N>
class Matrix
{
    static_assert(N > 1 && N <= 4, "Requires 1 < N <= 4");
    static_assert(M > 1 && M <= 4, "Requires 1 < M <= 4");

public:
    Vector<T, N> rows[M];

    //
    //  Constructor.
    //
    Matrix();

    //
    //  Construct from an arbitrarily-sized matrix.
    //
    template <typename U, size_t O, size_t P>
    Matrix(const Matrix<U, O, P>& cpy);

    //
    //  Assignment from an arbitrarily-sized matrix.
    //
    template <typename U, size_t O, size_t P>
    Matrix<T, M, N>& operator=(const Matrix<U, O, P>& rhs);

    //
    //  Array index operator.
    //
    Vector<T, N>& operator[](size_t i);
    Vector<T, N> operator[](size_t i) const;

    //
    //  Multiplication operator.
    //
    template <typename U, size_t O, size_t P>
    friend Matrix<T, M, P> operator*(const Matrix<T, M, N>& lhs, const Matrix<U, O, P>& rhs);

    //
    //  Transform a vector by a matrix.
    //
    template <size_t O>
    friend Vector<T, O> operator*(const Matrix<T, M, N>& lhs, const Vector<T, O>& rhs);

    //
    //  Returns an identity matrix.
    //
    static Matrix<T, M, N> Identity();

    //
    //  Returns a rotation matrix from a quaternion.
    //
    static Matrix<T, M, N> Rotation(const Vector<T, 4>& q);

    //
    //  Returns an affine translation matrix from a vector.
    //
    template <size_t O>
    static Matrix<T, M, N> Translation(const Vector<T, O>& v);

    //
    //  Returns a left-handed perspective projection matrix.
    //
    //  Parameters:
    //      [in] fov
    //          Field of view angle (in degrees.)
    //      [in] aspect
    //          Aspect ratio.
    //      [in] nearZ
    //          Near clipping plane.
    //      [in] farZ
    //          Far clipping plane.
    //
    static Matrix<T, M, N> PerspectiveProjection(const T& fov,
                                                 const T& aspect,
                                                 const T& nearZ,
                                                 const T& farZ);
};

template <typename T, size_t M, size_t N>
inline Matrix<T, M, N>::Matrix()
{
}

template <typename T, size_t M, size_t N> template <typename U, size_t O, size_t P>
inline Matrix<T, M, N>::Matrix(const Matrix<U, O, P>& cpy)
{
    *this = cpy;
}

template <typename T, size_t M, size_t N> template <typename U, size_t O, size_t P>
inline Matrix<T, M, N>& Matrix<T, M, N>::operator=(const Matrix<U, O, P>& rhs)
{
    for (size_t i = 0; i < M; i++)
    {
        if (i < O)
        {
            for (size_t j = 0; j < N; j++)
            {
                if (j < P)
                {
                    (*this)[i][j] = rhs[i][j];
                }
                else
                {
                    (*this)[i][j] = static_cast<T>((i == j) ? 1 : 0);
                }
            }
        }
        else
        {
            for (size_t j = 0; j < N; j++)
            {
                (*this)[i][j] = static_cast<T>((i == j) ? 1 : 0);
            }
        }
    }
}

template <typename T, size_t M, size_t N>
inline Vector<T, N>& Matrix<T, M, N>::operator[](size_t i)
{
    assert(i < M);
    return rows[i];
}

template <typename T, size_t M, size_t N>
inline Vector<T, N> Matrix<T, M, N>::operator[](size_t i) const
{
    assert(i < M);
    return rows[i];
}

template <typename T, size_t M, size_t N, typename U, size_t O, size_t P>
inline Matrix<T, M, P> operator*(const Matrix<T, M, N>& lhs, const Matrix<U, O, P>& rhs)
{
    static_assert(N == O, "Requires N == 0");

    Matrix<T, M, P> result;
    for (size_t i = 0; i < M; i++)
    {
        for (size_t j = 0; j < P; j++)
        {
            T value = 0;
            for (size_t k = 0; k < N; k++)
            {
                value += lhs[i][k] * rhs[k][j];
            }
            result[i][j] = value;
        }
    }
    return result;
}

template <typename T, size_t M, size_t N, size_t O>
inline Vector<T, O> operator*(const Matrix<T, M, N>& lhs, const Vector<T, O>& rhs)
{
    static_assert(O == N || O + 1 == N, "Requires O == N or O + 1 == N");
    Vector<T, N> result,
                 v(rhs);
    if (O < N)
    {
        v[N - 1] = static_cast<T>(1);
    }

    for (size_t i = 0; i < N; i++)
    {
        T value = 0;
        for (size_t j = 0; j < M; j++)
        {
            value += v[j] * lhs[i][j];
        }
        result[i] = value;
    }

    return Vector<T, O>(result);
}

template <typename T, size_t M, size_t N>
inline Matrix<T, M, N> Matrix<T, M, N>::Identity()
{
    Matrix<T, M, N> result;
    for (size_t i = 0; i < M; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            result[i][j] = static_cast<T>((i == j) ? 1 : 0);
        }
    }
    return result;
}

template <typename T, size_t M, size_t N>
inline Matrix<T, M, N> Matrix<T, M, N>::Rotation(const Vector<T, 4>& q)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    static_assert(M >= 3 && N >= 3, "Requires M >= 3 and N >= 3");
    Matrix<float, 4, 4> result;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result),
                    XMMatrixRotationQuaternion(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&q))));
    return Matrix<T, M, N>(result);
}

template <typename T, size_t M, size_t N> template <size_t O>
inline Matrix<T, M, N> Matrix<T, M, N>::Translation(const Vector<T, O>& v)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    static_assert(M == 4, "Requires M == 4");
    static_assert(N >= O, "Requires N >= O");
    Matrix<T, M, N> result = Matrix<T, M, N>::Identity();
    for (size_t i = 0; i < N; i++)
    {
        if (i < O)
        {
            result[3][i] = v[i];
        }
        else
        {
            result[3][i] = static_cast<T>((i == M - 1) ? 1 : 0);
        }
    }
    return result;
}

template <typename T, size_t M, size_t N>
inline Matrix<T, M, N> Matrix<T, M, N>::PerspectiveProjection(const T& fov,
                                                              const T& aspect,
                                                              const T& nearZ,
                                                              const T& farZ)
{
    static_assert(boost::is_float<T>::value, "Requires is_float<T>");
    static_assert(M == 4 && N == 4, "Requires 4x4 matrix");
    Matrix<float, M, N> result;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result),
                    XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ));
    return Matrix<T, M, N>(result);
}

typedef Matrix<float, 2, 2> float2x2;
typedef Matrix<float, 2, 3> float2x3;
typedef Matrix<float, 2, 4> float2x4;
typedef Matrix<float, 3, 2> float3x2;
typedef Matrix<float, 3, 3> float3x3;
typedef Matrix<float, 3, 4> float3x4;
typedef Matrix<float, 4, 2> float4x2;
typedef Matrix<float, 4, 3> float4x3;
typedef Matrix<float, 4, 4> float4x4;
typedef Matrix<double, 2, 2> double2x2;
typedef Matrix<double, 2, 3> double2x3;
typedef Matrix<double, 2, 4> double2x4;
typedef Matrix<double, 3, 2> double3x2;
typedef Matrix<double, 3, 3> double3x3;
typedef Matrix<double, 3, 4> double3x4;
typedef Matrix<double, 4, 2> double4x2;
typedef Matrix<double, 4, 3> double4x3;
typedef Matrix<double, 4, 4> double4x4;

#endif  // __NYX_MATRIX_H__
