//
//  base_vector3.h
//  base
//
//  Created by Tobias Schwandt on 23/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"
#include "base/base_math_operations.h"

#include <assert.h>
#include <math.h>

namespace MATH
{
    template <typename T>
    class CVector2;
    
    template <typename T>
    class CMatrix3x3;
    template <typename T>
    class CMatrix4x4;
} // namespace MATH

namespace MATH
{
    template<typename T>
    class CVector3
    {
    public:
        
        static const int s_MaxNumberOfElements = 3;
        
    public:
        
        typedef CVector3<T> CThis;
        typedef T           X;
        typedef X&          XRef;
        typedef const X&    XConstRef;
        typedef X*          XPtr;
        typedef const X*    XConstPtr;
        
        typedef CVector2<T> CThis2;

    public:

        static const CVector3<T> s_Zero;
        static const CVector3<T> s_One;
        static const CVector3<T> s_AxisX;
        static const CVector3<T> s_AxisY;
        static const CVector3<T> s_AxisZ;
        
    public:
        
        CVector3();
        CVector3(T _Value);
        CVector3(T _Value0, T _Value1);
        CVector3(T _Value0, T _Value1, T _Value2);
        CVector3(const CThis2& _rValue0, T _Value);
        CVector3(const CThis& _rCpy);
        
    public:
        
        inline void Set(T _Value);
        inline void Set(T _Value0, T _Value1);
        inline void Set(T _Value0, T _Value1, T _Value2);
        inline void Set(const CThis2& _rValue0, T _Value1);
        inline void Set(const CThis& _rVector);
        
        inline void SetZero();
        
    public:
        
        inline bool IsEqual(const CThis& _rVector, X _Epsilon) const;
        inline bool IsEqual(const CThis& _rVector, const CThis& _rEpsilon) const;
        
    public:
        
        inline CThis& operator = (CThis const& _rRight);
        
    public:
        
        inline T& operator [] (const int _Index);
        inline const T& operator [] (const int _Index) const;
        
    public:
        
        inline bool operator == (const CThis& _rRight) const;
        inline bool operator != (const CThis& _rRight) const;
        
    public:
        
        inline CThis operator - (const CThis& _rRight) const;
        inline CThis operator + (const CThis& _rRight) const;
        inline CThis operator * (const CThis& _rRight) const;
        inline CThis operator / (const CThis& _rRight) const;
        
        inline CThis operator * (const T _Scalar) const;
        inline CThis operator / (const T _Scalar) const;
        
        inline CThis operator * (const CMatrix3x3<T>& _rMatrix) const;
        inline CThis operator * (const CMatrix4x4<T>& _rMatrix) const;
        
    public:
        
        inline CThis& operator -= (const CThis& _rRight);
        inline CThis& operator += (const CThis& _rRight);
        inline CThis& operator *= (const CThis& _rRight);
        inline CThis& operator /= (const CThis& _rRight);
        
        inline CThis& operator *= (const T _Scalar);
        inline CThis& operator /= (const T _Scalar);
        
        inline CThis& operator *= (const CMatrix3x3<T>& _rMatrix);
        inline CThis& operator *= (const CMatrix4x4<T>& _rMatrix);
        
    public:
        
        inline T Length() const;
        
        inline CThis CrossProduct(CThis const& _rRight) const;
        inline T DotProduct(CThis const& _rRight) const;
        inline CThis Normalize() const;
        inline T SquaredLength() const;
        
    private:
        
        T m_V[s_MaxNumberOfElements];
        
    };
} // namespace MATH

namespace MATH
{
    typedef CVector3<unsigned char> Byte3;
    typedef CVector3<int>           Int3;
    typedef CVector3<unsigned int>  UInt3;
    typedef CVector3<float>         Float3;
    typedef CVector3<double>        Double3;
} // namespace MATH

namespace MATH
{
    template<typename T>
    const CVector3<T> CVector3<T>::s_Zero  = CVector3(T(0));

    template<typename T>
    const CVector3<T> CVector3<T>::s_One   = CVector3(T(1), T(1), T(1));

    template<typename T>
    const CVector3<T> CVector3<T>::s_AxisX = CVector3(T(1), T(0), T(0));

    template<typename T>
    const CVector3<T> CVector3<T>::s_AxisY = CVector3(T(0), T(1), T(0));

    template<typename T>
    const CVector3<T> CVector3<T>::s_AxisZ = CVector3(T(0), T(0), T(1));
} // namespace MATH

namespace MATH
{
    template<typename T>
    CVector3<T>::CVector3()
    {
        m_V[0] = 0;
        m_V[1] = 0;
        m_V[2] = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CVector3<T>::CVector3(T _Value)
    {
        m_V[0] = _Value;
        m_V[1] = _Value;
        m_V[2] = _Value;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CVector3<T>::CVector3(T _Value0, T _Value1)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CVector3<T>::CVector3(T _Value0, T _Value1, T _Value2)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = _Value2;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CVector3<T>::CVector3(const CThis2& _rValue0, T _Value)
    {
        m_V[0] = _rValue0[0];
        m_V[1] = _rValue0[1];
        m_V[2] = _Value;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CVector3<T>::CVector3(const CThis& _rCpy)
    {
        m_V[0] = _rCpy[0];
        m_V[1] = _rCpy[1];
        m_V[2] = _rCpy[2];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CVector3<T>::Set(T _Value)
    {
        m_V[0] = _Value;
        m_V[1] = _Value;
        m_V[2] = _Value;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CVector3<T>::Set(T _Value0, T _Value1)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = X(0);
    }
    
    // -----------------------------------------------------------------------------
    

    template<typename T>
    inline void CVector3<T>::Set(T _Value0, T _Value1, T _Value2)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = _Value2;
    }
    
    // -----------------------------------------------------------------------------
    

    template<typename T>
    inline void CVector3<T>::Set(const CThis2& _rValue0, T _Value1)
    {
        m_V[0] = _rValue0[0];
        m_V[1] = _rValue0[1];
        m_V[2] = _Value1;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CVector3<T>::Set(const CThis& _rVector)
    {
        m_V[0] = _rVector[0];
        m_V[1] = _rVector[1];
        m_V[2] = _rVector[2];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CVector3<T>::SetZero()
    {
        m_V[0] = X(0);
        m_V[1] = X(0);
        m_V[2] = X(0);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline bool CVector3<T>::IsEqual(const CThis& _rVector, X _Epsilon) const
    {
        return Base::IsEqual(m_V[0], _rVector[0], _Epsilon) && Base::IsEqual(m_V[1], _rVector[1], _Epsilon) && Base::IsEqual(m_V[2], _rVector[2], _Epsilon);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline bool CVector3<T>::IsEqual(const CThis& _rVector, const CThis& _rEpsilon) const
    {
        return Base::IsEqual(m_V[0], _rVector[0], _rEpsilon[0]) && Base::IsEqual(m_V[1], _rVector[1], _rEpsilon[1]) && Base::IsEqual(m_V[2], _rVector[2], _rEpsilon[2]);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator = (CThis const& _rRight)
    {
        m_V[0] = _rRight[0];
        m_V[1] = _rRight[1];
        m_V[2] = _rRight[2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline T& CVector3<T>::operator [] (const int _Index)
    {
        return m_V[_Index];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline const T& CVector3<T>::operator [] (const int _Index) const
    {
        return m_V[_Index];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline bool CVector3<T>::operator == (const CThis& _rRight) const
    {
        bool IsEqual;
        
        IsEqual = m_V[0] == _rRight.m_V[0] && m_V[1] == _rRight.m_V[1] && m_V[2] == _rRight.m_V[2];
        
        return IsEqual;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline bool CVector3<T>::operator != (const CThis& _rRight) const
    {
        bool IsEqual;
        
        IsEqual = m_V[0] != _rRight.m_V[0] || m_V[1] != _rRight.m_V[1] || m_V[2] != _rRight.m_V[2];
        
        return IsEqual;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator - (const CThis& _rRight) const
    {
        CThis MinThis;
        
        MinThis.m_V[0] = this->m_V[0] - _rRight.m_V[0];
        MinThis.m_V[1] = this->m_V[1] - _rRight.m_V[1];
        MinThis.m_V[2] = this->m_V[2] - _rRight.m_V[2];
        
        return MinThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator + (const CThis& _rRight) const
    {
        CThis AddThis;
        
        AddThis.m_V[0] = this->m_V[0] + _rRight.m_V[0];
        AddThis.m_V[1] = this->m_V[1] + _rRight.m_V[1];
        AddThis.m_V[2] = this->m_V[2] + _rRight.m_V[2];
        
        return AddThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator * (const CThis& _rRight) const
    {
        CThis MulThis;
        
        MulThis.m_V[0] = this->m_V[0] * _rRight.m_V[0];
        MulThis.m_V[1] = this->m_V[1] * _rRight.m_V[1];
        MulThis.m_V[2] = this->m_V[2] * _rRight.m_V[2];
        
        return MulThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator / (const CThis& _rRight) const
    {
        CThis DevThis;
        
        DevThis.m_V[0] = this->m_V[0] / _rRight.m_V[0];
        DevThis.m_V[1] = this->m_V[1] / _rRight.m_V[1];
        DevThis.m_V[2] = this->m_V[2] / _rRight.m_V[2];
        
        return DevThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator * (const T _Scalar) const
    {
        CThis MulThis;
        
        MulThis.m_V[0] = this->m_V[0] * _Scalar;
        MulThis.m_V[1] = this->m_V[1] * _Scalar;
        MulThis.m_V[2] = this->m_V[2] * _Scalar;
        
        return MulThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator / (const T _Scalar) const
    {
        CThis DevThis;
        
        DevThis.m_V[0] = this->m_V[0] / _Scalar;
        DevThis.m_V[1] = this->m_V[1] / _Scalar;
        DevThis.m_V[2] = this->m_V[2] / _Scalar;
        
        return DevThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator * (const CMatrix3x3<T>& _rMatrix) const
    {
        typedef CMatrix3x3<T> CMatrix;
        
        CThis MulThis;
        
        MulThis.m_V[0] = this->m_V[0] * _rMatrix.m_V[CMatrix::A11] + this->m_V[1] * _rMatrix.m_V[CMatrix::A21] + this->m_V[2] * _rMatrix.m_V[CMatrix::A31];
        MulThis.m_V[1] = this->m_V[0] * _rMatrix.m_V[CMatrix::A12] + this->m_V[1] * _rMatrix.m_V[CMatrix::A22] + this->m_V[2] * _rMatrix.m_V[CMatrix::A32];
        MulThis.m_V[2] = this->m_V[0] * _rMatrix.m_V[CMatrix::A13] + this->m_V[1] * _rMatrix.m_V[CMatrix::A23] + this->m_V[2] * _rMatrix.m_V[CMatrix::A33];
        
        return MulThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::operator * (const CMatrix4x4<T>& _rMatrix) const
    {
        typedef CMatrix4x4<T> CMatrix;
        
        CThis MulThis;
        
        MulThis.m_V[0] = this->m_V[0] * _rMatrix.m_V[CMatrix::A11] + this->m_V[1] * _rMatrix.m_V[CMatrix::A21] + this->m_V[2] * _rMatrix.m_V[CMatrix::A31] + _rMatrix.m_V[CMatrix::A41];
        MulThis.m_V[1] = this->m_V[0] * _rMatrix.m_V[CMatrix::A12] + this->m_V[1] * _rMatrix.m_V[CMatrix::A22] + this->m_V[2] * _rMatrix.m_V[CMatrix::A32] + _rMatrix.m_V[CMatrix::A42];
        MulThis.m_V[2] = this->m_V[0] * _rMatrix.m_V[CMatrix::A13] + this->m_V[1] * _rMatrix.m_V[CMatrix::A23] + this->m_V[2] * _rMatrix.m_V[CMatrix::A33] + _rMatrix.m_V[CMatrix::A43];
        
        return MulThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator -= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] - _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] - _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] - _rRight.m_V[2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator += (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] + _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] + _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] + _rRight.m_V[2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator *= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] * _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] * _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] * _rRight.m_V[2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator /= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] / _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] / _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] / _rRight.m_V[2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator *= (const T _Scalar)
    {
        this->m_V[0] = this->m_V[0] * _Scalar;
        this->m_V[1] = this->m_V[1] * _Scalar;
        this->m_V[2] = this->m_V[2] * _Scalar;
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator /= (const T _Scalar)
    {
        this->m_V[0] = this->m_V[0] / _Scalar;
        this->m_V[1] = this->m_V[1] / _Scalar;
        this->m_V[2] = this->m_V[2] / _Scalar;
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator *= (const CMatrix3x3<T>& _rMatrix)
    {
        this->m_V[0] = this->m_V[0] * _rMatrix[0][0] + this->m_V[0] * _rMatrix[0][1] + this->m_V[0] * _rMatrix[0][2];
        this->m_V[1] = this->m_V[0] * _rMatrix[1][0] + this->m_V[1] * _rMatrix[1][1] + this->m_V[1] * _rMatrix[1][2];
        this->m_V[2] = this->m_V[0] * _rMatrix[2][0] + this->m_V[2] * _rMatrix[2][1] + this->m_V[2] * _rMatrix[2][2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis& CVector3<T>::operator *= (const CMatrix4x4<T>& _rMatrix)
    {
        this->m_V[0] = this->m_V[0] * _rMatrix[0][0] + this->m_V[0] * _rMatrix[0][1] + this->m_V[0] * _rMatrix[0][2] + _rMatrix[0][3];
        this->m_V[1] = this->m_V[0] * _rMatrix[1][0] + this->m_V[1] * _rMatrix[1][1] + this->m_V[1] * _rMatrix[1][2] + _rMatrix[1][3];
        this->m_V[2] = this->m_V[0] * _rMatrix[2][0] + this->m_V[2] * _rMatrix[2][1] + this->m_V[2] * _rMatrix[2][2] + _rMatrix[2][3];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline T CVector3<T>::Length() const
    {
        return sqrt( (this->m_V[0] * this->m_V[0]) + (this->m_V[1] * this->m_V[1]) + (this->m_V[2] * this->m_V[2]) );
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::CrossProduct(CThis const& _rRight) const
    {
        X V[s_MaxNumberOfElements];

        V[0] = m_V[1] * _rRight.m_V[2] - m_V[2] * _rRight.m_V[1];
        V[1] = m_V[2] * _rRight.m_V[0] - m_V[0] * _rRight.m_V[2];
        V[2] = m_V[0] * _rRight.m_V[1] - m_V[1] * _rRight.m_V[0];

        return CThis(V[0], V[1], V[2]);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline T CVector3<T>::DotProduct(CThis const& _rRight) const
    {
        return this->m_V[0] * _rRight.m_V[0] + this->m_V[1] * _rRight.m_V[1] + this->m_V[2] * _rRight.m_V[2];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector3<T>::CThis CVector3<T>::Normalize() const
    {
        CThis Normalize;
        
        float Factor;
        
        T Length;
        
        Length = this->Length();
        
        assert(Length != 0);
        
        Factor = X(1) / Length;
        
        Normalize.m_V[0] = this->m_V[0] * Factor;
        Normalize.m_V[1] = this->m_V[1] * Factor;
        Normalize.m_V[2] = this->m_V[2] * Factor;
        
        return Normalize;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline T CVector3<T>::SquaredLength() const
    {
        return DotProduct(*this);
    }
} // namespace MATH

