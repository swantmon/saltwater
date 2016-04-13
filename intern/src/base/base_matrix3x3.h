
#pragma once

#include "base/base_defines.h"
#include "base/base_math_operations.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace MATH
{
    template<typename T>
    class CMatrix3x3
    {
    public:

        static const int s_MaxNumberOfColumns  = 3;
        static const int s_MaxNumberOfRows     = 3;
        static const int s_MaxNumberOfElements = s_MaxNumberOfColumns * s_MaxNumberOfRows;

    public:

        static const CMatrix3x3<T> s_Zero;
        static const CMatrix3x3<T> s_Identity;

    public:

        typedef CMatrix3x3<T> CThis;
        typedef CVector3<T>   CLine;
        typedef T             X;
        typedef T*            XPtr;
        typedef const T*      XConstPtr;
        typedef T&            XRef;
        typedef const T&      XConstRef;

    public:

        enum EIndices
        {
            A11 =  0, A12 =  1, A13 = 2,
            A21 =  3, A22 =  4, A23 = 5,
            A31 =  6, A32 =  7, A33 = 8
        };

        // -----------------------------------------------------------------------------
        // basic parts
        // -----------------------------------------------------------------------------

    public:

        inline CMatrix3x3();
        inline CMatrix3x3(X _Value);
        inline CMatrix3x3(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2);
        inline CMatrix3x3(X _11, X _12, X _13, X _21, X _22, X _23, X _31, X _32, X _33);
        inline CMatrix3x3(XConstPtr _pA);

        inline CMatrix3x3(const CThis& _rCpy);

    public:

        inline CThis& operator = (const CThis& _rRight);

    public:

        inline CLine& operator [] (const int _Index);
        inline const CLine& operator [] (const int _Index) const;

    public:

        inline CThis& SetRow(const int _Index, X _A0);
        inline CThis& SetRow(const int _Index, X _A0, X _A1, X _A2);
        inline CThis& SetRow(const int _Index, const CLine& _rVector);
        inline CLine& GetRow(const int _Index);
        inline const CLine& GetRow(const int _Index) const;

        inline CThis& SetColumn(const int _Index, X _A0);
        inline CThis& SetColumn(const int _Index, X _A0, X _A1, X _A2);
        inline CThis& SetColumn(const int _Index, const CLine& _rVector);
        inline CLine GetColumn(const int _Index) const;

    public:

        inline bool operator == (const CThis& _rRight) const;
        inline bool operator != (const CThis& _rRight) const;

    public:

        inline CThis operator + (const CThis& _rRight) const;
        inline CThis operator - (const CThis& _rRight) const;
        inline CThis operator * (const CThis& _rRight) const;

        inline CVector3<T> operator * (const CVector3<T>& _rVector) const;

        inline CThis operator * (X _Scalar) const;
        inline CThis operator / (X _Scalar) const;

        inline CThis& operator += (const CThis& _rRight) const;
        inline CThis& operator -= (const CThis& _rRight) const;
        inline CThis& operator *= (const CThis& _rRight) const;

        inline CThis& operator *= (X _Scalar) const;
        inline CThis& operator /= (X _Scalar) const;

    public:

        inline CThis& SetZero();
        inline CThis& SetIdentity();

        inline CThis& Set(X _Value);
        inline CThis& Set(const CThis& _rRight);
        inline CThis& Set(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2);
        inline CThis& Set(X _11, X _12, X _13, X _21, X _22, X _23, X _31, X _32, X _33);
        inline CThis& Set(XConstPtr _pA);
        
    public:
        
        inline CThis& Transpose();
        inline CThis GetTransposed() const;
        
        inline CThis& Invert();
        inline CThis GetInverted() const;

    public:

        // -----------------------------------------------------------------------------

        inline CThis& SetScale(X _Scale);
        inline CThis& SetScale(X _X, X _Y, X _Z);
        inline CThis& SetScale(CVector3<T>& _rVector);
        inline CThis& SetScale(const CLine& _rVector);

        inline void GetScale(X& _rX, X& _rY, X& _rZ) const;
        inline void GetScale(CLine& _rScales) const;

        // -----------------------------------------------------------------------------

        inline CThis& SetRotationX(X _AngleInRadiens);
        inline CThis& SetRotationY(X _AngleInRadiens);
        inline CThis& SetRotationZ(X _AngleInRadiens);
        inline CThis& SetRotation(X _AngleInRadiensX, X _AngleInRadiensY, X _AngleInRadiensZ);

        inline void GetRotation(CMatrix3x3<T>& _rRotationMatrix) const;
        
        inline void GetRotation(CVector3<T>& _rRotation);
        
    public:
        
        inline CThis& LookAt(const Float3& _rEye, const Float3& _rTarget, const Float3& _rUp);
        
    public:
        
        bool IsEqual(const CThis& _rRight, X _Epsilon) const;

    private:

        T m_V[s_MaxNumberOfElements];
        
    private:
        
        friend class CVector3<T>;
        friend class CVector4<T>;

    };
} // namespace MATH


namespace MATH
{

    typedef CMatrix3x3<int>    Int3x3;
    typedef CMatrix3x3<float>  Float3x3;
    typedef CMatrix3x3<double> Double3x3;

} // namespace MATH


namespace MATH
{

    template<typename T>
    const CMatrix3x3<T> CMatrix3x3<T>::s_Zero     = CMatrix3x3(T(0));

    template<typename T>
    const CMatrix3x3<T> CMatrix3x3<T>::s_Identity = CMatrix3x3(
        T(1), T(0), T(0), 
        T(0), T(1), T(0), 
        T(0), T(0), T(1));

} // namespace MATH


namespace MATH
{
    template<typename T>
    inline CMatrix3x3<T>::CMatrix3x3()
    {
        Set(X(0));
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline CMatrix3x3<T>::CMatrix3x3(X _Value)
    {
        Set(_Value);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline CMatrix3x3<T>::CMatrix3x3(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2)
    {
        Set(_rLine0, _rLine1, _rLine2);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline CMatrix3x3<T>::CMatrix3x3(X _11, X _12, X _13, X _21, X _22, X _23, X _31, X _32, X _33)
    {
        Set(_11, _12, _13, _21, _22, _23, _31, _32, _33);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline CMatrix3x3<T>::CMatrix3x3(XConstPtr _pA)
    {
        Set(_pA);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CMatrix3x3<T>::CMatrix3x3(const CThis& _rCpy)
    {
        Set(_rCpy);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::operator = (const CThis& _rRight)
    {
        Set(_rRight);

        return *this;
    }

} // namespace MATH

namespace MATH
{
    template<typename T>
    inline typename CMatrix3x3<T>::CLine& CMatrix3x3<T>::operator [] (const int _Index)
    {
        return GetRow(_Index);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline const typename CMatrix3x3<T>::CLine& CMatrix3x3<T>::operator [] (const int _Index) const
    {
        return GetRow(_Index);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRow(const int _Index, X _A0)
    {
        SetRow(_Index, _A0, _A0, _A0);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRow(const int _Index, X _A0, X _A1, X _A2)
    {
        assert(_Index < s_MaxNumberOfColumns);

        m_V[_Index + 0] = _A0;
        m_V[_Index + 1] = _A1;
        m_V[_Index + 2] = _A2;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRow(const int _Index, const CLine& _rVector)
    {
        SetRow(_Index, _rVector[0], _rVector[1], _rVector[2]);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CLine& CMatrix3x3<T>::GetRow(const int _Index)
    {
        return *reinterpret_cast<CLine*>(&m_V[_Index * s_MaxNumberOfColumns]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline const typename CMatrix3x3<T>::CLine& CMatrix3x3<T>::GetRow(const int _Index) const
    {
        return *reinterpret_cast<const CLine*>(&m_V[_Index * s_MaxNumberOfColumns]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetColumn(const int _Index, X _A0)
    {
        SetColumn(_Index, _A0, _A0, _A0);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetColumn(const int _Index, X _A0, X _A1, X _A2)
    {
        assert(_Index < s_MaxNumberOfRows);

        m_V[_Index +  0] = _A0;
        m_V[_Index +  4] = _A1;
        m_V[_Index +  8] = _A2;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetColumn(const int _Index, const CLine& _rVector)
    {
        SetColumn(_Index, _rVector[0], _rVector[1], _rVector[2]);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CLine CMatrix3x3<T>::GetColumn(const int _Index) const
    {
        assert(_Index < s_MaxNumberOfRows);

        T Temp[CLine::s_MaxNumberOfElements];

        Temp[0] = m_V[_Index +  0];
        Temp[1] = m_V[_Index +  4];
        Temp[2] = m_V[_Index +  8];

        return CLine(Temp[0], Temp[1], Temp[2]);
    }

} // namespace MATH

namespace MATH
{

    template<typename T>
    inline bool CMatrix3x3<T>::operator == (const CThis& _rRight) const
    {
        bool IsEqual;

        IsEqual = ((*this)[0] == _rRight[0]) && ((*this)[1] == _rRight[1]) && ((*this)[2] == _rRight[2]);

        return IsEqual;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline bool CMatrix3x3<T>::operator != (const CThis& _rRight) const
    {
        bool IsDifferent;

        IsDifferent = ((*this)[0] != _rRight[0]) || ((*this)[1] != _rRight[1]) || ((*this)[2] != _rRight[2]);

        return IsDifferent;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::operator + (const CThis& _rRight) const
    {
        CThis AddThis;

        AddThis[0] = (*this)[0] + _rRight[0];
        AddThis[1] = (*this)[1] + _rRight[1];
        AddThis[2] = (*this)[2] + _rRight[2];

        return AddThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::operator - (const CThis& _rRight) const
    {
        CThis MinThis;

        MinThis[0] = (*this)[0] - _rRight[0];
        MinThis[1] = (*this)[1] - _rRight[1];
        MinThis[2] = (*this)[2] - _rRight[2];

        return MinThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::operator * (const CThis& _rRight) const
    {
        CThis MulThis;

        MulThis.m_V[A11] = m_V[A11] * _rRight.m_V[A11] + m_V[A12] * _rRight.m_V[A21] + m_V[A13] * _rRight.m_V[A31];
        MulThis.m_V[A12] = m_V[A11] * _rRight.m_V[A12] + m_V[A12] * _rRight.m_V[A22] + m_V[A13] * _rRight.m_V[A32];
        MulThis.m_V[A13] = m_V[A11] * _rRight.m_V[A13] + m_V[A12] * _rRight.m_V[A23] + m_V[A13] * _rRight.m_V[A33];

        MulThis.m_V[A21] = m_V[A21] * _rRight.m_V[A11] + m_V[A22] * _rRight.m_V[A21] + m_V[A23] * _rRight.m_V[A31];
        MulThis.m_V[A22] = m_V[A21] * _rRight.m_V[A12] + m_V[A22] * _rRight.m_V[A22] + m_V[A23] * _rRight.m_V[A32];
        MulThis.m_V[A23] = m_V[A21] * _rRight.m_V[A13] + m_V[A22] * _rRight.m_V[A23] + m_V[A23] * _rRight.m_V[A33];

        MulThis.m_V[A31] = m_V[A31] * _rRight.m_V[A11] + m_V[A32] * _rRight.m_V[A21] + m_V[A33] * _rRight.m_V[A31];
        MulThis.m_V[A32] = m_V[A31] * _rRight.m_V[A12] + m_V[A32] * _rRight.m_V[A22] + m_V[A33] * _rRight.m_V[A32];
        MulThis.m_V[A33] = m_V[A31] * _rRight.m_V[A13] + m_V[A32] * _rRight.m_V[A23] + m_V[A33] * _rRight.m_V[A33];

        return MulThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline CVector3<T> CMatrix3x3<T>::operator * (const CVector3<T>& _rVector) const
    {
        CVector3<T> MulThis;

        MulThis[0] = m_V[A11] * _rVector[0] + m_V[A12] * _rVector[1] + m_V[A13] * _rVector[2];
        MulThis[1] = m_V[A21] * _rVector[0] + m_V[A22] * _rVector[1] + m_V[A23] * _rVector[2];
        MulThis[2] = m_V[A31] * _rVector[0] + m_V[A32] * _rVector[1] + m_V[A33] * _rVector[2];

        return MulThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::operator * (X _Scalar) const
    {
        CThis ScalarThis;

        ScalarThis[0] = (*this)[0] * _Scalar;
        ScalarThis[1] = (*this)[1] * _Scalar;
        ScalarThis[2] = (*this)[2] * _Scalar;

        return ScalarThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::operator / (X _Scalar) const
    {
        CThis ScalarThis;

        ScalarThis[0] = (*this)[0] / _Scalar;
        ScalarThis[1] = (*this)[1] / _Scalar;
        ScalarThis[2] = (*this)[2] / _Scalar;

        return ScalarThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::operator += (const CThis& _rRight) const
    {
        (*this)[0] += _rRight[0];
        (*this)[1] += _rRight[1];
        (*this)[2] += _rRight[2];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::operator -= (const CThis& _rRight) const
    {
        (*this)[0] -= _rRight[0];
        (*this)[1] -= _rRight[1];
        (*this)[2] -= _rRight[2];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::operator *= (const CThis& _rRight) const
    {
        return Set((*this) * _rRight);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::operator *= (X _Scalar) const
    {
        (*this)[0] *= _Scalar;
        (*this)[1] *= _Scalar;
        (*this)[2] *= _Scalar;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::operator /= (X _Scalar) const
    {
        (*this)[0] /= _Scalar;
        (*this)[1] /= _Scalar;
        (*this)[2] /= _Scalar;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetZero()
    {
        return Set(s_Zero);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetIdentity()
    {
        return Set(s_Identity);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Set(X _Value)
    {
        m_V[A11] = _Value;
        m_V[A12] = _Value;
        m_V[A13] = _Value;

        m_V[A21] = _Value;
        m_V[A22] = _Value;
        m_V[A23] = _Value;

        m_V[A31] = _Value;
        m_V[A32] = _Value;
        m_V[A33] = _Value;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Set(const CThis& _rRight)
    {
        m_V[A11] = _rRight.m_V[A11];
        m_V[A12] = _rRight.m_V[A12];
        m_V[A13] = _rRight.m_V[A13];

        m_V[A21] = _rRight.m_V[A21];
        m_V[A22] = _rRight.m_V[A22];
        m_V[A23] = _rRight.m_V[A23];

        m_V[A31] = _rRight.m_V[A31];
        m_V[A32] = _rRight.m_V[A32];
        m_V[A33] = _rRight.m_V[A33];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Set(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2)
    {
        m_V[A11] = _rLine0[0];
        m_V[A12] = _rLine0[1];
        m_V[A13] = _rLine0[2];

        m_V[A21] = _rLine1[0];
        m_V[A22] = _rLine1[1];
        m_V[A23] = _rLine1[2];

        m_V[A31] = _rLine2[0];
        m_V[A32] = _rLine2[1];
        m_V[A33] = _rLine2[2];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Set(X _11, X _12, X _13, X _21, X _22, X _23, X _31, X _32, X _33)
    {
        m_V[A11] = _11;
        m_V[A12] = _12;
        m_V[A13] = _13;

        m_V[A21] = _21;
        m_V[A22] = _22;
        m_V[A23] = _23;

        m_V[A31] = _31;
        m_V[A32] = _32;
        m_V[A33] = _33;

        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Set(XConstPtr _pA)
    {
        assert(_pA != 0);
        
        m_V[ 0] = _pA[ 0]; m_V[ 1] = _pA[ 1]; m_V[ 2] = _pA[ 2];
        m_V[ 3] = _pA[ 3]; m_V[ 4] = _pA[ 4]; m_V[ 5] = _pA[ 5];
        m_V[ 6] = _pA[ 6]; m_V[ 7] = _pA[ 7]; m_V[ 8] = _pA[ 8];
        
        return *this;
    }
    
    // ------------------------------------------------------------------------------

    template <typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Transpose()
    {
        Base::Swap(m_V[A21], m_V[A12]);
        Base::Swap(m_V[A31], m_V[A13]);
        Base::Swap(m_V[A32], m_V[A23]);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::GetTransposed() const
    {
        X A[s_MaxNumberOfElements];
        
        A[A11] = m_V[A11]; A[A12] = m_V[A21]; A[A13] = m_V[A31];
        A[A21] = m_V[A12]; A[A22] = m_V[A22]; A[A23] = m_V[A32];
        A[A31] = m_V[A13]; A[A32] = m_V[A23]; A[A33] = m_V[A33];
        
        return CThis(A);
    }
    
    // ------------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::Invert()
    {
        float Determinant;
        float ReciprocalDeterminant;
        X A[s_MaxNumberOfElements];
        
        A[A11] = m_V[A22] * m_V[A33] - m_V[A23] * m_V[A32];
        A[A12] = m_V[A13] * m_V[A32] - m_V[A12] * m_V[A33];
        A[A13] = m_V[A12] * m_V[A23] - m_V[A13] * m_V[A22];
        A[A21] = m_V[A23] * m_V[A31] - m_V[A21] * m_V[A33];
        A[A22] = m_V[A11] * m_V[A33] - m_V[A13] * m_V[A31];
        A[A23] = m_V[A13] * m_V[A21] - m_V[A11] * m_V[A23];
        A[A31] = m_V[A21] * m_V[A32] - m_V[A22] * m_V[A31];
        A[A32] = m_V[A12] * m_V[A31] - m_V[A11] * m_V[A32];
        A[A33] = m_V[A11] * m_V[A22] - m_V[A12] * m_V[A21];
        
        // ------------------------------------------------------------------------------
        // Calculate the determinant.
        // ------------------------------------------------------------------------------
        Determinant = m_V[A11] * A[A11] + m_V[A12] * A[A21] + m_V[A13] * A[A31];
        
        assert(Determinant != X(0));
        
        ReciprocalDeterminant = X(1) / Determinant;
        
        m_V[A11] = A[A11] * ReciprocalDeterminant; m_V[A12] = A[A12] * ReciprocalDeterminant; m_V[A13] = A[A13] * ReciprocalDeterminant;
        m_V[A21] = A[A21] * ReciprocalDeterminant; m_V[A22] = A[A22] * ReciprocalDeterminant; m_V[A23] = A[A23] * ReciprocalDeterminant;
        m_V[A31] = A[A31] * ReciprocalDeterminant; m_V[A32] = A[A32] * ReciprocalDeterminant; m_V[A33] = A[A33] * ReciprocalDeterminant;
        
        return *this;
    }
    
    // ------------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix3x3<T>::CThis CMatrix3x3<T>::GetInverted() const
    {
        X Determinant;
        X ReciprocalDeterminant;
        X A[s_MaxNumberOfElements];
        
        A[A11] = m_V[A22] * m_V[A33] - m_V[A23] * m_V[A32];
        A[A12] = m_V[A13] * m_V[A32] - m_V[A12] * m_V[A33];
        A[A13] = m_V[A12] * m_V[A23] - m_V[A13] * m_V[A22];
        A[A21] = m_V[A23] * m_V[A31] - m_V[A21] * m_V[A33];
        A[A22] = m_V[A11] * m_V[A33] - m_V[A13] * m_V[A31];
        A[A23] = m_V[A13] * m_V[A21] - m_V[A11] * m_V[A23];
        A[A31] = m_V[A21] * m_V[A32] - m_V[A22] * m_V[A31];
        A[A32] = m_V[A12] * m_V[A31] - m_V[A11] * m_V[A32];
        A[A33] = m_V[A11] * m_V[A22] - m_V[A12] * m_V[A21];
        
        // ------------------------------------------------------------------------------
        // Calculate the determinant.
        // ------------------------------------------------------------------------------
        Determinant = m_V[A11] * A[A11] + m_V[A12] * A[A21] + m_V[A13] * A[A31];
        
        assert(Determinant != X(0));
        
        ReciprocalDeterminant = X(1) / Determinant;
        
        A[A11] *= ReciprocalDeterminant; A[A12] *= ReciprocalDeterminant; A[A13] *= ReciprocalDeterminant;
        A[A21] *= ReciprocalDeterminant; A[A22] *= ReciprocalDeterminant; A[A23] *= ReciprocalDeterminant;
        A[A31] *= ReciprocalDeterminant; A[A32] *= ReciprocalDeterminant; A[A33] *= ReciprocalDeterminant;
        
        return CThis(A);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetScale(X _Scale)
    {
        return SetScale(_Scale, _Scale, _Scale);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetScale(X _X, X _Y, X _Z)
    {
        SetIdentity();

        m_V[A11] = _X;
        m_V[A22] = _Y;
        m_V[A33] = _Z;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetScale(CVector3<T>& _rVector)
    {
        return SetScale(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------


    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetScale(const CLine& _rVector)
    {
        return SetScale(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix3x3<T>::GetScale(X& _rX, X& _rY, X& _rZ) const
    {
        _rX = m_V[A11];
        _rY = m_V[A22];
        _rZ = m_V[A33];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix3x3<T>::GetScale(CLine& _rScales) const
    {
        _rScales[0] = m_V[A11];
        _rScales[1] = m_V[A22];
        _rScales[2] = m_V[A33];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRotationX(X _AngleInRadiens)
    {
        X Cos = std::cos(_AngleInRadiens);
        X Sin = std::sin(_AngleInRadiens);

        m_V[A11] = 1; m_V[A12] = 0;   m_V[A13] = 0;
        m_V[A21] = 0; m_V[A22] = Cos; m_V[A23] = -Sin;
        m_V[A31] = 0; m_V[A32] = Sin; m_V[A33] =  Cos;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRotationY(X _AngleInRadiens)
    {
        X Cos = std::cos(_AngleInRadiens);
        X Sin = std::sin(_AngleInRadiens);

        m_V[A11] = Cos;  m_V[A12] = 0; m_V[A13] = Sin;
        m_V[A21] = 0;    m_V[A22] = 1; m_V[A23] = 0;
        m_V[A31] = -Sin; m_V[A32] = 0; m_V[A33] = Cos;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRotationZ(X _AngleInRadiens)
    {
        X Cos = std::cos(_AngleInRadiens);
        X Sin = std::sin(_AngleInRadiens);

        m_V[A11] = Cos; m_V[A12] = -Sin; m_V[A13] = 0;
        m_V[A21] = Sin; m_V[A22] =  Cos; m_V[A23] = 0;
        m_V[A31] = 0;   m_V[A32] = 0;    m_V[A33] = 1;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::SetRotation(X _AngleInRadiensX, X _AngleInRadiensY, X _AngleInRadiensZ)
    {
        // X-axis (pitch)
        X SinX = std::sin(_AngleInRadiensX);
        X CosX = std::cos(_AngleInRadiensX);

        // Y-axis (yaw)
        X SinY = std::sin(_AngleInRadiensY);
        X CosY = std::cos(_AngleInRadiensY);

        // Z-axis (roll)
        X SinZ = std::sin(_AngleInRadiensZ);
        X CosZ = std::cos(_AngleInRadiensZ);

        m_V[A11] =  CosY * CosZ; m_V[A12] =  SinX * SinY * CosZ + CosX * SinZ; m_V[A13] = -CosX * SinY * CosZ + SinX * SinZ;
        m_V[A21] = -CosY * SinZ; m_V[A22] = -SinX * SinY * SinZ + CosX * CosZ; m_V[A23] =  CosX * SinY * SinZ + SinX * CosZ;
        m_V[A31] =  SinY;        m_V[A32] = -SinX * CosY;                      m_V[A33] =  CosX * CosY;

        return *this;

    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix3x3<T>::GetRotation(CMatrix3x3<T>& _rRotationMatrix) const
    {
        _rRotationMatrix[A11] = m_V[A11]; _rRotationMatrix[A12] = m_V[A12]; _rRotationMatrix[A13] = m_V[A13];
        _rRotationMatrix[A21] = m_V[A21]; _rRotationMatrix[A22] = m_V[A22]; _rRotationMatrix[A23] = m_V[A23];
        _rRotationMatrix[A31] = m_V[A31]; _rRotationMatrix[A32] = m_V[A32]; _rRotationMatrix[A33] = m_V[A33];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CMatrix3x3<T>::GetRotation(CVector3<T>& _rRotation)
    {
        _rRotation[0] = atan2(m_V[A32], m_V[A33]);
        _rRotation[1] = atan2(-m_V[A31], sqrt(m_V[A32] * m_V[A32] + m_V[A33] * m_V[A33]));
        _rRotation[2] = atan2(m_V[A21], m_V[A11]);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix3x3<T>::CThis& CMatrix3x3<T>::LookAt(const Float3& _rEye, const Float3& _rTarget, const Float3& _rUp)
    {
        // -----------------------------------------------------------------------------
        // This is from:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/bb281711(v=vs.85).aspx
        // -----------------------------------------------------------------------------
        Base::Float3 LookDirection = (_rEye - _rTarget).Normalize();
        Base::Float3 Up            = _rUp.Normalize();
        Base::Float3 Right         = LookDirection.CrossProduct(Up).Normalize();

        Up = LookDirection.CrossProduct(Right);
         
        m_V[A11] =          Right[0]; m_V[A12] =          Right[1]; m_V[A13] =          Right[2];
        m_V[A21] =             Up[0]; m_V[A22] =             Up[1]; m_V[A23] =             Up[2];
        m_V[A31] = -LookDirection[0]; m_V[A32] = -LookDirection[1]; m_V[A33] = -LookDirection[2];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CMatrix3x3<T>::IsEqual(const CThis& _rRight, X _Epsilon) const
    {
        return MATH::IsEqual(m_V[A11], _rRight.m_V[A11], _Epsilon)
            && MATH::IsEqual(m_V[A12], _rRight.m_V[A12], _Epsilon)
            && MATH::IsEqual(m_V[A13], _rRight.m_V[A13], _Epsilon)
            && MATH::IsEqual(m_V[A21], _rRight.m_V[A21], _Epsilon)
            && MATH::IsEqual(m_V[A22], _rRight.m_V[A22], _Epsilon)
            && MATH::IsEqual(m_V[A23], _rRight.m_V[A23], _Epsilon)
            && MATH::IsEqual(m_V[A31], _rRight.m_V[A31], _Epsilon)
            && MATH::IsEqual(m_V[A32], _rRight.m_V[A32], _Epsilon)
            && MATH::IsEqual(m_V[A33], _rRight.m_V[A33], _Epsilon);
    }
} // namespace MATH