
#pragma once

#include "base/base_defines.h"
#include "base/base_math_operations.h"
#include "base/base_matrix3x3.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

namespace MATH
{
    template<typename T>
    class CMatrix4x4
    {
    public:

        static const int s_MaxNumberOfColumns  = 4;
        static const int s_MaxNumberOfRows     = 4;
        static const int s_MaxNumberOfElements = s_MaxNumberOfColumns * s_MaxNumberOfRows;

    public:

        static const CMatrix4x4<T> s_Zero;
        static const CMatrix4x4<T> s_Identity;

    public:

        typedef CMatrix4x4<T> CThis;
        typedef CMatrix3x3<T> CLittleBrother;
        typedef CVector4<T>   CLine;
        typedef T             X;
        typedef T*            XPtr;
        typedef const T*      XConstPtr;
        typedef T&            XRef;
        typedef const T&      XConstRef;

    public:

        //
        //   Row-Major: Set-function use this view
        //   | A11, A12, A13, A14 |
        //   | A21, A22, A23, A24 |
        //   | A32, A32, A33, A34 |
        //   | A41, A42, A43, A44 |
        //  
        //   Column-Major: Values inside memory
        //   | A11, A21, A31, A41 |
        //   | A12, A22, A32, A42 |
        //   | A13, A23, A33, A43 |
        //   | A14, A24, A34, A44 | 
        //

        enum EIndices
        {
            A11 =  0, A12 =  1, A13 =  2, A14 =  3,
            A21 =  4, A22 =  5, A23 =  6, A24 =  7,
            A31 =  8, A32 =  9, A33 = 10, A34 = 11,
            A41 = 12, A42 = 13, A43 = 14, A44 = 15
        };

        // -----------------------------------------------------------------------------
        // basic parts
        // -----------------------------------------------------------------------------
        
    public:

        inline CMatrix4x4();
        inline CMatrix4x4(X _Value);
        inline CMatrix4x4(const CMatrix3x3<T>& _rRight);
        inline CMatrix4x4(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2, const CLine& _rLine3);
        inline CMatrix4x4(X _11, X _12, X _13, X _14, X _21, X _22, X _23, X _24, X _31, X _32, X _33, X _34, X _41, X _42, X _43, X _44);
        inline CMatrix4x4(XConstPtr _pA);

        inline CMatrix4x4(const CThis& _rCpy);

    public:

        inline CThis& operator = (const CThis& _rRight);
        inline CThis& operator = (const CMatrix3x3<T>& _rRight);

    public:

        inline CLine& operator [] (unsigned int _Index);
        inline const CLine& operator [] (unsigned int _Index) const;
        
        inline void SetRow(unsigned int _Index, X _A0, X _A1, X _A2, X _A3);
        inline CLine& GetRow(unsigned int _Index);
        inline const CLine& GetRow(unsigned int _Index) const;
        
        inline void SetColumn(unsigned int _Index, X _A0, X _A1, X _A2, X _A3);
        inline CLine GetColumn(unsigned int _Index) const;

    public:

        inline bool operator == (const CThis& _rRight) const;
        inline bool operator != (const CThis& _rRight) const;

    public:

        inline CThis operator + (const CThis& _rRight) const;
        inline CThis operator - (const CThis& _rRight) const;
        inline CThis operator * (const CThis& _rRight) const;

        inline CVector3<T> operator * (const CVector3<T>& _rVector) const;
        inline CVector4<T> operator * (const CVector4<T>& _rVector) const;

        inline CThis operator * (X _Scalar) const;
        inline CThis operator / (X _Scalar) const;

        inline CThis& operator += (const CThis& _rRight);
        inline CThis& operator -= (const CThis& _rRight);
        inline CThis& operator *= (const CThis& _rRight);

        inline CThis& operator *= (X _Scalar);
        inline CThis& operator /= (X _Scalar);

    public:

        inline CThis& SetZero();
        inline CThis& SetIdentity();

        inline CThis& Set(X _Value);
        inline CThis& Set(const CThis& _rRight);
        inline CThis& Set(const CMatrix3x3<T>& _rRight);
        inline CThis& Set(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2, const CLine& _rLine3);
        inline CThis& Set(X _11, X _12, X _13, X _14, X _21, X _22, X _23, X _24, X _31, X _32, X _33, X _34, X _41, X _42, X _43, X _44);
        inline CThis& Set(XConstPtr _pA);
        
    public:
        
        inline CThis& Transpose();
        inline CThis GetTransposed() const;
        
        inline CThis& Invert();
        inline CThis GetInverted() const;

    public:

        inline CThis& InjectTranslation(X _AxisX, X _AxisY, X _AxisZ);
        inline CThis& InjectTranslation(CVector3<T>& _rVector);
        inline CThis& InjectTranslation(const CLine& _rVector);

        inline CThis& SetTranslation(X _AxisX, X _AxisY, X _AxisZ);
        inline CThis& SetTranslation(CVector3<T>& _rVector);
        inline CThis& SetTranslation(const CLine& _rVector);

        inline void GetTranslation(X& _rX, X& _rY, X& _rZ) const;
        inline void GetTranslation(CVector3<T>& _rTranslations) const;
        inline void GetTranslation(CLine& _rTranslations) const;

        // -----------------------------------------------------------------------------

        inline CThis& InjectScale(X _Scale);
        inline CThis& InjectScale(X _AxisX, X _AxisY, X _AxisZ);
        inline CThis& InjectScale(CVector3<T>& _rVector);
        inline CThis& InjectScale(const CLine& _rVector);

        inline CThis& SetScale(X _Scale);
        inline CThis& SetScale(X _AxisX, X _AxisY, X _AxisZ);
        inline CThis& SetScale(const CVector3<T>& _rVector);
        inline CThis& SetScale(const CLine& _rVector);

        inline void GetScale(X& _rX, X& _rY, X& _rZ) const;
        inline void GetScale(CVector3<T>& _rScales) const;
        inline void GetScale(CLine& _rScales) const;

        // -----------------------------------------------------------------------------

        inline CThis& InjectRotationX(X _AngleInRadiens);
        inline CThis& InjectRotationY(X _AngleInRadiens);
        inline CThis& InjectRotationZ(X _AngleInRadiens);
        inline CThis& InjectRotation(X _AngleInRadiensX, X _AngleInRadiensY, X _AngleInRadiensZ);

        inline CThis& SetRotationX(X _AngleInRadiens);
        inline CThis& SetRotationY(X _AngleInRadiens);
        inline CThis& SetRotationZ(X _AngleInRadiens);
        inline CThis& SetRotation(X _AngleInRadiensX, X _AngleInRadiensY, X _AngleInRadiensZ);

        inline void GetRotation(CMatrix3x3<T>& _rRotationMatrix) const;

        inline void GetRotation(CVector3<T>& _rRotation);
        
    public:
        
        inline CThis& LookAt(const Float3& _rEye, const Float3& _rTarget, const Float3& _rUp);

    public:

        inline CThis& SetRHPerspective(X _Width, X _Height, X _Near, X _Far);
        inline CThis& SetRHPerspective(X _Left, X _Right, X _Bottom, X _Top, X _Near, X _Far);
        inline CThis& SetRHPerspective(X _Near, X _Far, Base::CMatrix3x3<X> _CameraMatrix);
        inline CThis& SetRHFieldOfView(X _FOVY, X _Aspect, X _Near, X _Far);
        inline CThis& SetRHOrthographic(X _Width, X _Height, X _Near, X _Far);
        inline CThis& SetRHOrthographic(X _Left, X _Right, X _Bottom, X _Top, X _Near, X _Far);

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

    typedef CMatrix4x4<int>    Int4x4;
    typedef CMatrix4x4<float>  Float4x4;
    typedef CMatrix4x4<double> Double4x4;

} // namespace MATH


namespace MATH
{

	template<typename T>
    const CMatrix4x4<T> CMatrix4x4<T>::s_Zero     = CMatrix4x4(T(0));

    template<typename T>
    const CMatrix4x4<T> CMatrix4x4<T>::s_Identity = CMatrix4x4(
        T(1), T(0), T(0), T(0), 
        T(0), T(1), T(0), T(0), 
        T(0), T(0), T(1), T(0), 
        T(0), T(0), T(0), T(1));

} // namespace MATH


namespace MATH
{
    template<typename T>
    CMatrix4x4<T>::CMatrix4x4()
    {
        Set(s_Identity);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CMatrix4x4<T>::CMatrix4x4(X _Value)
    {
        Set(_Value);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline CMatrix4x4<T>::CMatrix4x4(const CMatrix3x3<T>& _rRight)
    {
        Set(_rRight);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CMatrix4x4<T>::CMatrix4x4(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2, const CLine& _rLine3)
    {
        Set(_rLine0, _rLine1, _rLine2, _rLine3);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CMatrix4x4<T>::CMatrix4x4(X _11, X _12, X _13, X _14, X _21, X _22, X _23, X _24, X _31, X _32, X _33, X _34, X _41, X _42, X _43, X _44)
    {
        Set(_11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline CMatrix4x4<T>::CMatrix4x4(XConstPtr _pA)
    {
        Set(_pA);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CMatrix4x4<T>::CMatrix4x4(const CThis& _rCpy)
    {
        Set(_rCpy);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator = (const CThis& _rRight)
    {
        Set(_rRight);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator = (const CMatrix3x3<T>& _rRight)
    {
        Set(_rRight);

        return *this;
    }

} // namespace MATH

namespace MATH
{
    template <typename T>
    inline typename CMatrix4x4<T>::CLine& CMatrix4x4<T>::operator [] (unsigned int _Index)
    {
        return GetRow(_Index);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline const typename CMatrix4x4<T>::CLine& CMatrix4x4<T>::operator [] (unsigned int _Index) const
    {
        return GetRow(_Index);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline void CMatrix4x4<T>::SetRow(unsigned int _Index, X _A0, X _A1, X _A2, X _A3)
    {
        assert(_Index < 4);
        
        GetRow(_Index).Set(_A0, _A1, _A2, _A3);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CLine& CMatrix4x4<T>::GetRow(unsigned int _Index)
    {
        assert(_Index < 4);
        
        return *reinterpret_cast<CLine*>(&m_V[_Index * 4]);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline const typename CMatrix4x4<T>::CLine& CMatrix4x4<T>::GetRow(unsigned int _Index) const
    {
        assert(_Index < 4);
        
        return *reinterpret_cast<const CLine*>(&m_V[_Index * 4]);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline void CMatrix4x4<T>::SetColumn(unsigned int _Index, X _A0, X _A1, X _A2, X _A3)
    {
        assert(_Index < 4);
        
        m_V[ 0 + _Index] = _A0;
        m_V[ 4 + _Index] = _A1;
        m_V[ 8 + _Index] = _A2;
        m_V[12 + _Index] = _A3;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CLine CMatrix4x4<T>::GetColumn(unsigned int _Index) const
    {
        X V[CLine::s_NumberOfComponents];
        
        assert(_Index < 4);
        
        V[0] = m_V[ 0 + _Index];
        V[1] = m_V[ 4 + _Index];
        V[2] = m_V[ 8 + _Index];
        V[3] = m_V[12 + _Index];
        
        return CLine(V[0], V[1], V[2], V[3]);
    }
} // namespace MATH

namespace MATH
{

    template<typename T>
    inline bool CMatrix4x4<T>::operator == (const CThis& _rRight) const
    {
        bool IsEqual;

        IsEqual = ((*this)[0] == _rRight[0]) && ((*this)[1] == _rRight[1]) && ((*this)[2] == _rRight[2]) && ((*this)[3] == _rRight[3]);

        return IsEqual;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline bool CMatrix4x4<T>::operator != (const CThis& _rRight) const
    {
        bool IsDifferent;

        IsDifferent = ((*this)[0] != _rRight[0]) || ((*this)[1] != _rRight[1]) || ((*this)[2] != _rRight[2]) || ((*this)[3] != _rRight[3]);

        return IsDifferent;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::operator + (const CThis& _rRight) const
    {
        CThis AddThis;

        AddThis[0] = (*this)[0] + _rRight[0];
        AddThis[1] = (*this)[1] + _rRight[1];
        AddThis[2] = (*this)[2] + _rRight[2];
        AddThis[3] = (*this)[3] + _rRight[3];

        return AddThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::operator - (const CThis& _rRight) const
    {
        CThis MinThis;

        MinThis[0] = (*this)[0] - _rRight[0];
        MinThis[1] = (*this)[1] - _rRight[1];
        MinThis[2] = (*this)[2] - _rRight[2];
        MinThis[3] = (*this)[3] - _rRight[3];

        return MinThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::operator * (const CThis& _rRight) const
    {
        X A[16];
        
        A[A11] = m_V[A11] * _rRight.m_V[A11] + m_V[A12] * _rRight.m_V[A21] + m_V[A13] * _rRight.m_V[A31] + m_V[A14] * _rRight.m_V[A41];
        A[A12] = m_V[A11] * _rRight.m_V[A12] + m_V[A12] * _rRight.m_V[A22] + m_V[A13] * _rRight.m_V[A32] + m_V[A14] * _rRight.m_V[A42];
        A[A13] = m_V[A11] * _rRight.m_V[A13] + m_V[A12] * _rRight.m_V[A23] + m_V[A13] * _rRight.m_V[A33] + m_V[A14] * _rRight.m_V[A43];
        A[A14] = m_V[A11] * _rRight.m_V[A14] + m_V[A12] * _rRight.m_V[A24] + m_V[A13] * _rRight.m_V[A34] + m_V[A14] * _rRight.m_V[A44];
        A[A21] = m_V[A21] * _rRight.m_V[A11] + m_V[A22] * _rRight.m_V[A21] + m_V[A23] * _rRight.m_V[A31] + m_V[A24] * _rRight.m_V[A41];
        A[A22] = m_V[A21] * _rRight.m_V[A12] + m_V[A22] * _rRight.m_V[A22] + m_V[A23] * _rRight.m_V[A32] + m_V[A24] * _rRight.m_V[A42];
        A[A23] = m_V[A21] * _rRight.m_V[A13] + m_V[A22] * _rRight.m_V[A23] + m_V[A23] * _rRight.m_V[A33] + m_V[A24] * _rRight.m_V[A43];
        A[A24] = m_V[A21] * _rRight.m_V[A14] + m_V[A22] * _rRight.m_V[A24] + m_V[A23] * _rRight.m_V[A34] + m_V[A24] * _rRight.m_V[A44];
        A[A31] = m_V[A31] * _rRight.m_V[A11] + m_V[A32] * _rRight.m_V[A21] + m_V[A33] * _rRight.m_V[A31] + m_V[A34] * _rRight.m_V[A41];
        A[A32] = m_V[A31] * _rRight.m_V[A12] + m_V[A32] * _rRight.m_V[A22] + m_V[A33] * _rRight.m_V[A32] + m_V[A34] * _rRight.m_V[A42];
        A[A33] = m_V[A31] * _rRight.m_V[A13] + m_V[A32] * _rRight.m_V[A23] + m_V[A33] * _rRight.m_V[A33] + m_V[A34] * _rRight.m_V[A43];
        A[A34] = m_V[A31] * _rRight.m_V[A14] + m_V[A32] * _rRight.m_V[A24] + m_V[A33] * _rRight.m_V[A34] + m_V[A34] * _rRight.m_V[A44];
        A[A41] = m_V[A41] * _rRight.m_V[A11] + m_V[A42] * _rRight.m_V[A21] + m_V[A43] * _rRight.m_V[A31] + m_V[A44] * _rRight.m_V[A41];
        A[A42] = m_V[A41] * _rRight.m_V[A12] + m_V[A42] * _rRight.m_V[A22] + m_V[A43] * _rRight.m_V[A32] + m_V[A44] * _rRight.m_V[A42];
        A[A43] = m_V[A41] * _rRight.m_V[A13] + m_V[A42] * _rRight.m_V[A23] + m_V[A43] * _rRight.m_V[A33] + m_V[A44] * _rRight.m_V[A43];
        A[A44] = m_V[A41] * _rRight.m_V[A14] + m_V[A42] * _rRight.m_V[A24] + m_V[A43] * _rRight.m_V[A34] + m_V[A44] * _rRight.m_V[A44];
        
        return CThis(A);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline CVector3<T> CMatrix4x4<T>::operator * (const CVector3<T>& _rVector) const
    {
        CVector3<T> MulThis;

        MulThis[0] = m_V[A11] * _rVector[0] + m_V[A12] * _rVector[1] + m_V[A13] * _rVector[2] + m_V[A14];
        MulThis[1] = m_V[A21] * _rVector[0] + m_V[A22] * _rVector[1] + m_V[A23] * _rVector[2] + m_V[A24];
        MulThis[2] = m_V[A31] * _rVector[0] + m_V[A32] * _rVector[1] + m_V[A33] * _rVector[2] + m_V[A34];

        return MulThis;
    }

    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline CVector4<T> CMatrix4x4<T>::operator * (const CVector4<T>& _rVector) const
    {
        CVector4<T> MulThis;

        MulThis[0] = m_V[A11] * _rVector[0] + m_V[A12] * _rVector[1] + m_V[A13] * _rVector[2] + m_V[A14] * _rVector[3];
        MulThis[1] = m_V[A21] * _rVector[0] + m_V[A22] * _rVector[1] + m_V[A23] * _rVector[2] + m_V[A24] * _rVector[3];
        MulThis[2] = m_V[A31] * _rVector[0] + m_V[A32] * _rVector[1] + m_V[A33] * _rVector[2] + m_V[A34] * _rVector[3];
        MulThis[3] = m_V[A41] * _rVector[0] + m_V[A42] * _rVector[1] + m_V[A43] * _rVector[2] + m_V[A44] * _rVector[3];

        return MulThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::operator * (X _Scalar) const
    {
        CThis ScalarThis;

        ScalarThis[0] = (*this)[0] * _Scalar;
        ScalarThis[1] = (*this)[1] * _Scalar;
        ScalarThis[2] = (*this)[2] * _Scalar;
        ScalarThis[3] = (*this)[3] * _Scalar;

        return ScalarThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::operator / (X _Scalar) const
    {
        CThis ScalarThis;

        ScalarThis[0] = (*this)[0] / _Scalar;
        ScalarThis[1] = (*this)[1] / _Scalar;
        ScalarThis[2] = (*this)[2] / _Scalar;
        ScalarThis[3] = (*this)[3] / _Scalar;

        return ScalarThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator += (const CThis& _rRight)
    {
        (*this)[0] += _rRight[0];
        (*this)[1] += _rRight[1];
        (*this)[2] += _rRight[2];
        (*this)[3] += _rRight[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator -= (const CThis& _rRight)
    {
        (*this)[0] -= _rRight[0];
        (*this)[1] -= _rRight[1];
        (*this)[2] -= _rRight[2];
        (*this)[3] -= _rRight[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator *= (const CThis& _rRight)
    {
        return this->Set((*this) * _rRight);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator *= (X _Scalar)
    {
        (*this)[0] *= _Scalar;
        (*this)[1] *= _Scalar;
        (*this)[2] *= _Scalar;
        (*this)[3] *= _Scalar;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::operator /= (X _Scalar)
    {
        (*this)[0] /= _Scalar;
        (*this)[1] /= _Scalar;
        (*this)[2] /= _Scalar;
        (*this)[3] /= _Scalar;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetZero()
    {
        return Set(s_Zero);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetIdentity()
    {
        return Set(s_Identity);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Set(X _Value)
    {
        m_V[A11] = _Value;
        m_V[A12] = _Value;
        m_V[A13] = _Value;
        m_V[A14] = _Value;

        m_V[A21] = _Value;
        m_V[A22] = _Value;
        m_V[A23] = _Value;
        m_V[A24] = _Value;

        m_V[A31] = _Value;
        m_V[A32] = _Value;
        m_V[A33] = _Value;
        m_V[A34] = _Value;

        m_V[A41] = _Value;
        m_V[A42] = _Value;
        m_V[A43] = _Value;
        m_V[A44] = _Value;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Set(const CThis& _rRight)
    {
        m_V[A11] = _rRight.m_V[A11];
        m_V[A12] = _rRight.m_V[A12];
        m_V[A13] = _rRight.m_V[A13];
        m_V[A14] = _rRight.m_V[A14];

        m_V[A21] = _rRight.m_V[A21];
        m_V[A22] = _rRight.m_V[A22];
        m_V[A23] = _rRight.m_V[A23];
        m_V[A24] = _rRight.m_V[A24];

        m_V[A31] = _rRight.m_V[A31];
        m_V[A32] = _rRight.m_V[A32];
        m_V[A33] = _rRight.m_V[A33];
        m_V[A34] = _rRight.m_V[A34];

        m_V[A41] = _rRight.m_V[A41];
        m_V[A42] = _rRight.m_V[A42];
        m_V[A43] = _rRight.m_V[A43];
        m_V[A44] = _rRight.m_V[A44];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Set(const CMatrix3x3<T>& _rRight)
    {
        m_V[A11] = _rRight[0][0]; m_V[A12] = _rRight[0][1]; m_V[A13] = _rRight[0][2]; m_V[A14] = 0;
        m_V[A21] = _rRight[1][0]; m_V[A22] = _rRight[1][1]; m_V[A23] = _rRight[1][2]; m_V[A24] = 0;
        m_V[A31] = _rRight[2][0]; m_V[A32] = _rRight[2][1]; m_V[A33] = _rRight[2][2]; m_V[A34] = 0;
        m_V[A41] = 0;              m_V[A42] = 0;              m_V[A43] = 0;              m_V[A44] = 1;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Set(const CLine& _rLine0, const CLine& _rLine1, const CLine& _rLine2, const CLine& _rLine3)
    {
        m_V[A11] = _rLine0[0];
        m_V[A12] = _rLine0[1];
        m_V[A13] = _rLine0[2];
        m_V[A14] = _rLine0[3];

        m_V[A21] = _rLine1[0];
        m_V[A22] = _rLine1[1];
        m_V[A23] = _rLine1[2];
        m_V[A24] = _rLine1[3];

        m_V[A31] = _rLine2[0];
        m_V[A32] = _rLine2[1];
        m_V[A33] = _rLine2[2];
        m_V[A34] = _rLine2[3];

        m_V[A41] = _rLine3[0];
        m_V[A42] = _rLine3[1];
        m_V[A43] = _rLine3[2];
        m_V[A44] = _rLine3[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Set(X _11, X _12, X _13, X _14, X _21, X _22, X _23, X _24, X _31, X _32, X _33, X _34, X _41, X _42, X _43, X _44)
    {
        m_V[A11] = _11;
        m_V[A12] = _12;
        m_V[A13] = _13;
        m_V[A14] = _14;

        m_V[A21] = _21;
        m_V[A22] = _22;
        m_V[A23] = _23;
        m_V[A24] = _24;

        m_V[A31] = _31;
        m_V[A32] = _32;
        m_V[A33] = _33;
        m_V[A34] = _34;

        m_V[A41] = _41;
        m_V[A42] = _42;
        m_V[A43] = _43;
        m_V[A44] = _44;

        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Set(XConstPtr _pA)
    {
        assert(_pA != 0);
        
        m_V[ 0] = _pA[ 0]; m_V[ 1] = _pA[ 1]; m_V[ 2] = _pA[ 2]; m_V[ 3] = _pA[ 3];
        m_V[ 4] = _pA[ 4]; m_V[ 5] = _pA[ 5]; m_V[ 6] = _pA[ 6]; m_V[ 7] = _pA[ 7];
        m_V[ 8] = _pA[ 8]; m_V[ 9] = _pA[ 9]; m_V[10] = _pA[10]; m_V[11] = _pA[11];
        m_V[12] = _pA[12]; m_V[13] = _pA[13]; m_V[14] = _pA[14]; m_V[15] = _pA[15];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Transpose()
    {
        Base::Swap(m_V[A21], m_V[A12]);
        Base::Swap(m_V[A31], m_V[A13]);
        Base::Swap(m_V[A32], m_V[A23]);
        Base::Swap(m_V[A41], m_V[A14]);
        Base::Swap(m_V[A42], m_V[A24]);
        Base::Swap(m_V[A43], m_V[A34]);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::GetTransposed() const
    {
        X A[s_MaxNumberOfElements];
        
        A[A11] = m_V[A11]; A[A12] = m_V[A21]; A[A13] = m_V[A31]; A[A14] = m_V[A41];
        A[A21] = m_V[A12]; A[A22] = m_V[A22]; A[A23] = m_V[A32]; A[A24] = m_V[A42];
        A[A31] = m_V[A13]; A[A32] = m_V[A23]; A[A33] = m_V[A33]; A[A34] = m_V[A43];
        A[A41] = m_V[A14]; A[A42] = m_V[A24]; A[A43] = m_V[A34]; A[A44] = m_V[A44];
        
        return CThis(A);
    }
    
    // ------------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::Invert()
    {
        float Determinant;
        float ReciprocalDeterminant;
        X A[s_MaxNumberOfElements];
        
        A[A11] =  m_V[A22] * m_V[A33] * m_V[A44] + m_V[A23] * m_V[A34] * m_V[A42] + m_V[A24] * m_V[A32] * m_V[A43] - m_V[A22] * m_V[A34] * m_V[A43] - m_V[A23] * m_V[A32] * m_V[A44] - m_V[A24] * m_V[A33] * m_V[A42];
        A[A12] =  m_V[A12] * m_V[A34] * m_V[A43] + m_V[A13] * m_V[A32] * m_V[A44] + m_V[A14] * m_V[A33] * m_V[A42] - m_V[A12] * m_V[A33] * m_V[A44] - m_V[A13] * m_V[A34] * m_V[A42] - m_V[A14] * m_V[A32] * m_V[A43];
        A[A13] =  m_V[A12] * m_V[A23] * m_V[A44] + m_V[A13] * m_V[A24] * m_V[A42] + m_V[A14] * m_V[A22] * m_V[A43] - m_V[A12] * m_V[A24] * m_V[A43] - m_V[A13] * m_V[A22] * m_V[A44] - m_V[A14] * m_V[A23] * m_V[A42];
        A[A14] =  m_V[A12] * m_V[A24] * m_V[A33] + m_V[A13] * m_V[A22] * m_V[A34] + m_V[A14] * m_V[A23] * m_V[A32] - m_V[A12] * m_V[A23] * m_V[A34] - m_V[A13] * m_V[A24] * m_V[A32] - m_V[A14] * m_V[A22] * m_V[A33];

        A[A21] =  m_V[A21] * m_V[A34] * m_V[A43] + m_V[A23] * m_V[A31] * m_V[A44] + m_V[A24] * m_V[A33] * m_V[A41] - m_V[A21] * m_V[A33] * m_V[A44] - m_V[A23] * m_V[A34] * m_V[A41] - m_V[A24] * m_V[A31] * m_V[A43];
        A[A22] =  m_V[A11] * m_V[A33] * m_V[A44] + m_V[A13] * m_V[A34] * m_V[A41] + m_V[A14] * m_V[A31] * m_V[A43] - m_V[A11] * m_V[A34] * m_V[A43] - m_V[A13] * m_V[A31] * m_V[A44] - m_V[A14] * m_V[A33] * m_V[A41];
        A[A23] =  m_V[A11] * m_V[A24] * m_V[A43] + m_V[A13] * m_V[A21] * m_V[A44] + m_V[A14] * m_V[A23] * m_V[A41] - m_V[A11] * m_V[A23] * m_V[A44] - m_V[A13] * m_V[A24] * m_V[A41] - m_V[A14] * m_V[A21] * m_V[A43];
        A[A24] =  m_V[A11] * m_V[A23] * m_V[A34] + m_V[A13] * m_V[A24] * m_V[A31] + m_V[A14] * m_V[A21] * m_V[A33] - m_V[A11] * m_V[A24] * m_V[A33] - m_V[A13] * m_V[A21] * m_V[A34] - m_V[A14] * m_V[A23] * m_V[A31];

        A[A31] =  m_V[A21] * m_V[A32] * m_V[A44] + m_V[A22] * m_V[A34] * m_V[A41] + m_V[A24] * m_V[A31] * m_V[A42] - m_V[A21] * m_V[A34] * m_V[A42] - m_V[A22] * m_V[A31] * m_V[A44] - m_V[A24] * m_V[A32] * m_V[A41];
        A[A32] =  m_V[A11] * m_V[A34] * m_V[A42] + m_V[A12] * m_V[A31] * m_V[A44] + m_V[A14] * m_V[A32] * m_V[A41] - m_V[A11] * m_V[A32] * m_V[A44] - m_V[A12] * m_V[A34] * m_V[A41] - m_V[A14] * m_V[A31] * m_V[A42];
        A[A33] =  m_V[A11] * m_V[A22] * m_V[A44] + m_V[A12] * m_V[A24] * m_V[A41] + m_V[A14] * m_V[A21] * m_V[A42] - m_V[A11] * m_V[A24] * m_V[A42] - m_V[A12] * m_V[A21] * m_V[A44] - m_V[A14] * m_V[A22] * m_V[A41];
        A[A34] =  m_V[A11] * m_V[A24] * m_V[A32] + m_V[A12] * m_V[A21] * m_V[A34] + m_V[A14] * m_V[A22] * m_V[A31] - m_V[A11] * m_V[A22] * m_V[A34] - m_V[A12] * m_V[A24] * m_V[A31] - m_V[A14] * m_V[A21] * m_V[A32];

        A[A41] =  m_V[A21] * m_V[A33] * m_V[A42] + m_V[A22] * m_V[A31] * m_V[A43] + m_V[A23] * m_V[A32] * m_V[A41] - m_V[A21] * m_V[A32] * m_V[A43] - m_V[A22] * m_V[A33] * m_V[A41] - m_V[A23] * m_V[A31] * m_V[A42];
        A[A42] =  m_V[A11] * m_V[A32] * m_V[A43] + m_V[A12] * m_V[A33] * m_V[A41] + m_V[A13] * m_V[A31] * m_V[A42] - m_V[A11] * m_V[A33] * m_V[A42] - m_V[A12] * m_V[A31] * m_V[A43] - m_V[A13] * m_V[A32] * m_V[A41];
        A[A43] =  m_V[A11] * m_V[A23] * m_V[A42] + m_V[A12] * m_V[A21] * m_V[A43] + m_V[A13] * m_V[A22] * m_V[A41] - m_V[A11] * m_V[A22] * m_V[A43] - m_V[A12] * m_V[A23] * m_V[A41] - m_V[A13] * m_V[A21] * m_V[A42];
        A[A44] =  m_V[A11] * m_V[A22] * m_V[A33] + m_V[A12] * m_V[A23] * m_V[A31] + m_V[A13] * m_V[A21] * m_V[A32] - m_V[A11] * m_V[A23] * m_V[A32] - m_V[A12] * m_V[A21] * m_V[A33] - m_V[A13] * m_V[A22] * m_V[A31];

        // ------------------------------------------------------------------------------
        // Calculate the determinant.
        // ------------------------------------------------------------------------------
        Determinant = m_V[A11] * A[A11] + m_V[A12] * A[A21] + m_V[A13] * A[A31] + m_V[A14] * A[A41];
        
        assert(Determinant != X(0));
        
        ReciprocalDeterminant = X(1) / Determinant;
        
        m_V[A11] = A[A11] * ReciprocalDeterminant; m_V[A12] = A[A12] * ReciprocalDeterminant; m_V[A13] = A[A13] * ReciprocalDeterminant; m_V[A14] = A[A14] * ReciprocalDeterminant;
        m_V[A21] = A[A21] * ReciprocalDeterminant; m_V[A22] = A[A22] * ReciprocalDeterminant; m_V[A23] = A[A23] * ReciprocalDeterminant; m_V[A24] = A[A24] * ReciprocalDeterminant;
        m_V[A31] = A[A31] * ReciprocalDeterminant; m_V[A32] = A[A32] * ReciprocalDeterminant; m_V[A33] = A[A33] * ReciprocalDeterminant; m_V[A34] = A[A34] * ReciprocalDeterminant;
        m_V[A41] = A[A41] * ReciprocalDeterminant; m_V[A42] = A[A42] * ReciprocalDeterminant; m_V[A43] = A[A43] * ReciprocalDeterminant; m_V[A44] = A[A44] * ReciprocalDeterminant;
        
        return *this;
    }
    
    // ------------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis CMatrix4x4<T>::GetInverted() const
    {
        X Determinant;
        X ReciprocalDeterminant;
        X A[s_MaxNumberOfElements];
        
        A[A11] =  m_V[A22] * m_V[A33] * m_V[A44] + m_V[A23] * m_V[A34] * m_V[A42] + m_V[A24] * m_V[A32] * m_V[A43] - m_V[A22] * m_V[A34] * m_V[A43] - m_V[A23] * m_V[A32] * m_V[A44] - m_V[A24] * m_V[A33] * m_V[A42];
        A[A12] =  m_V[A12] * m_V[A34] * m_V[A43] + m_V[A13] * m_V[A32] * m_V[A44] + m_V[A14] * m_V[A33] * m_V[A42] - m_V[A12] * m_V[A33] * m_V[A44] - m_V[A13] * m_V[A34] * m_V[A42] - m_V[A14] * m_V[A32] * m_V[A43];
        A[A13] =  m_V[A12] * m_V[A23] * m_V[A44] + m_V[A13] * m_V[A24] * m_V[A42] + m_V[A14] * m_V[A22] * m_V[A43] - m_V[A12] * m_V[A24] * m_V[A43] - m_V[A13] * m_V[A22] * m_V[A44] - m_V[A14] * m_V[A23] * m_V[A42];
        A[A14] =  m_V[A12] * m_V[A24] * m_V[A33] + m_V[A13] * m_V[A22] * m_V[A34] + m_V[A14] * m_V[A23] * m_V[A32] - m_V[A12] * m_V[A23] * m_V[A34] - m_V[A13] * m_V[A24] * m_V[A32] - m_V[A14] * m_V[A22] * m_V[A33];
        
        A[A21] =  m_V[A21] * m_V[A34] * m_V[A43] + m_V[A23] * m_V[A31] * m_V[A44] + m_V[A24] * m_V[A33] * m_V[A41] - m_V[A21] * m_V[A33] * m_V[A44] - m_V[A23] * m_V[A34] * m_V[A41] - m_V[A24] * m_V[A31] * m_V[A43];
        A[A22] =  m_V[A11] * m_V[A33] * m_V[A44] + m_V[A13] * m_V[A34] * m_V[A41] + m_V[A14] * m_V[A31] * m_V[A43] - m_V[A11] * m_V[A34] * m_V[A43] - m_V[A13] * m_V[A31] * m_V[A44] - m_V[A14] * m_V[A33] * m_V[A41];
        A[A23] =  m_V[A11] * m_V[A24] * m_V[A43] + m_V[A13] * m_V[A21] * m_V[A44] + m_V[A14] * m_V[A23] * m_V[A41] - m_V[A11] * m_V[A23] * m_V[A44] - m_V[A13] * m_V[A24] * m_V[A41] - m_V[A14] * m_V[A21] * m_V[A43];
        A[A24] =  m_V[A11] * m_V[A23] * m_V[A34] + m_V[A13] * m_V[A24] * m_V[A31] + m_V[A14] * m_V[A21] * m_V[A33] - m_V[A11] * m_V[A24] * m_V[A33] - m_V[A13] * m_V[A21] * m_V[A34] - m_V[A14] * m_V[A23] * m_V[A31];
        
        A[A31] =  m_V[A21] * m_V[A32] * m_V[A44] + m_V[A22] * m_V[A34] * m_V[A41] + m_V[A24] * m_V[A31] * m_V[A42] - m_V[A21] * m_V[A34] * m_V[A42] - m_V[A22] * m_V[A31] * m_V[A44] - m_V[A24] * m_V[A32] * m_V[A41];
        A[A32] =  m_V[A11] * m_V[A34] * m_V[A42] + m_V[A12] * m_V[A31] * m_V[A44] + m_V[A14] * m_V[A32] * m_V[A41] - m_V[A11] * m_V[A32] * m_V[A44] - m_V[A12] * m_V[A34] * m_V[A41] - m_V[A14] * m_V[A31] * m_V[A42];
        A[A33] =  m_V[A11] * m_V[A22] * m_V[A44] + m_V[A12] * m_V[A24] * m_V[A41] + m_V[A14] * m_V[A21] * m_V[A42] - m_V[A11] * m_V[A24] * m_V[A42] - m_V[A12] * m_V[A21] * m_V[A44] - m_V[A14] * m_V[A22] * m_V[A41];
        A[A34] =  m_V[A11] * m_V[A24] * m_V[A32] + m_V[A12] * m_V[A21] * m_V[A34] + m_V[A14] * m_V[A22] * m_V[A31] - m_V[A11] * m_V[A22] * m_V[A34] - m_V[A12] * m_V[A24] * m_V[A31] - m_V[A14] * m_V[A21] * m_V[A32];
        
        A[A41] =  m_V[A21] * m_V[A33] * m_V[A42] + m_V[A22] * m_V[A31] * m_V[A43] + m_V[A23] * m_V[A32] * m_V[A41] - m_V[A21] * m_V[A32] * m_V[A43] - m_V[A22] * m_V[A33] * m_V[A41] - m_V[A23] * m_V[A31] * m_V[A42];
        A[A42] =  m_V[A11] * m_V[A32] * m_V[A43] + m_V[A12] * m_V[A33] * m_V[A41] + m_V[A13] * m_V[A31] * m_V[A42] - m_V[A11] * m_V[A33] * m_V[A42] - m_V[A12] * m_V[A31] * m_V[A43] - m_V[A13] * m_V[A32] * m_V[A41];
        A[A43] =  m_V[A11] * m_V[A23] * m_V[A42] + m_V[A12] * m_V[A21] * m_V[A43] + m_V[A13] * m_V[A22] * m_V[A41] - m_V[A11] * m_V[A22] * m_V[A43] - m_V[A12] * m_V[A23] * m_V[A41] - m_V[A13] * m_V[A21] * m_V[A42];
        A[A44] =  m_V[A11] * m_V[A22] * m_V[A33] + m_V[A12] * m_V[A23] * m_V[A31] + m_V[A13] * m_V[A21] * m_V[A32] - m_V[A11] * m_V[A23] * m_V[A32] - m_V[A12] * m_V[A21] * m_V[A33] - m_V[A13] * m_V[A22] * m_V[A31];
        
        // ------------------------------------------------------------------------------
        // Calculate the determinant.
        // ------------------------------------------------------------------------------
        Determinant = m_V[A11] * A[A11] + m_V[A12] * A[A21] + m_V[A13] * A[A31] + m_V[A14] * A[A41];
        
        assert(Determinant != X(0));
        
        ReciprocalDeterminant = X(1) / Determinant;
        
        A[A11] *= ReciprocalDeterminant; A[A12] *= ReciprocalDeterminant; A[A13] *= ReciprocalDeterminant; A[A14] *= ReciprocalDeterminant;
        A[A21] *= ReciprocalDeterminant; A[A22] *= ReciprocalDeterminant; A[A23] *= ReciprocalDeterminant; A[A24] *= ReciprocalDeterminant;
        A[A31] *= ReciprocalDeterminant; A[A32] *= ReciprocalDeterminant; A[A33] *= ReciprocalDeterminant; A[A34] *= ReciprocalDeterminant;
        A[A41] *= ReciprocalDeterminant; A[A42] *= ReciprocalDeterminant; A[A43] *= ReciprocalDeterminant; A[A44] *= ReciprocalDeterminant;
        
        return CThis(A);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectTranslation(X _AxisX, X _AxisY, X _AxisZ)
    {
        m_V[A14] = _AxisX;
        m_V[A24] = _AxisY;
        m_V[A34] = _AxisZ;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectTranslation(CVector3<T>& _rVector)
    {
        return InjectTranslation(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectTranslation(const CLine& _rVector)
    {
        return InjectTranslation(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetTranslation(X _AxisX, X _AxisY, X _AxisZ)
    {
        SetIdentity();

        return InjectTranslation(_AxisX, _AxisY, _AxisZ);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetTranslation(CVector3<T>& _rVector)
    {
        return SetTranslation(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetTranslation(const CLine& _rVector)
    {
        return SetTranslation(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetTranslation(X& _rX, X& _rY, X& _rZ) const
    {
        _rX = m_V[A14];
        _rY = m_V[A24];
        _rZ = m_V[A34];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetTranslation(CVector3<T>& _rTranslations) const
    {
        _rTranslations[0] = m_V[A14];
        _rTranslations[1] = m_V[A24];
        _rTranslations[2] = m_V[A34];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetTranslation(CLine& _rTranslations) const
    {
        _rTranslations[0] = m_V[A14];
        _rTranslations[1] = m_V[A24];
        _rTranslations[2] = m_V[A34];
        _rTranslations[3] = X(1);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectScale(X _Scale)
    {
        return InjectScale(_Scale, _Scale, _Scale);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectScale(X _AxisX, X _AxisY, X _AxisZ)
    {
        m_V[A11] = _AxisX;
        m_V[A22] = _AxisY;
        m_V[A33] = _AxisZ;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectScale(CVector3<T>& _rVector)
    {
        return InjectScale(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectScale(const CLine& _rVector)
    {
        return InjectScale(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetScale(X _Scale)
    {
        return SetScale(_Scale, _Scale, _Scale);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetScale(X _AxisX, X _AxisY, X _AxisZ)
    {
        SetIdentity();

        return InjectScale(_AxisX, _AxisY, _AxisZ);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetScale(const CVector3<T>& _rVector)
    {
        return SetScale(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------


    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetScale(const CLine& _rVector)
    {
        return SetScale(_rVector[0], _rVector[1], _rVector[2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetScale(X& _rX, X& _rY, X& _rZ) const
    {
        _rX = m_V[A11];
        _rY = m_V[A22];
        _rZ = m_V[A33];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetScale(CVector3<T>& _rScales) const
    {
        _rScales[0] = m_V[A11];
        _rScales[1] = m_V[A22];
        _rScales[2] = m_V[A33];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetScale(CLine& _rScales) const
    {
        _rScales[0] = m_V[A11];
        _rScales[1] = m_V[A22];
        _rScales[2] = m_V[A33];
        _rScales[3] = X(1);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectRotationX(X _AngleInRadiens)
    {
        X Cos = std::cos(_AngleInRadiens);
        X Sin = std::sin(_AngleInRadiens);

        m_V[A11] = X(1); m_V[A12] = X(0); m_V[A13] = X(0);
        m_V[A21] = X(0); m_V[A22] =  Cos; m_V[A23] = -Sin;
        m_V[A31] = X(0); m_V[A32] =  Sin; m_V[A33] =  Cos;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectRotationY(X _AngleInRadiens)
    {
        X Cos = std::cos(_AngleInRadiens);
        X Sin = std::sin(_AngleInRadiens);

        m_V[A11] =  Cos; m_V[A12] = X(0); m_V[A13] =  Sin;
        m_V[A21] = X(0); m_V[A22] = X(1); m_V[A23] = X(0);
        m_V[A31] = -Sin; m_V[A32] = X(0); m_V[A33] =  Cos;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectRotationZ(X _AngleInRadiens)
    {
        X Cos = std::cos(_AngleInRadiens);
        X Sin = std::sin(_AngleInRadiens);

        m_V[A11] =  Cos; m_V[A12] = -Sin; m_V[A13] = X(0);
        m_V[A21] =  Sin; m_V[A22] =  Cos; m_V[A23] = X(0);
        m_V[A31] = X(0); m_V[A32] = X(0); m_V[A33] = X(1);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::InjectRotation(X _AngleInRadiensX, X _AngleInRadiensY, X _AngleInRadiensZ)
    {
        // -----------------------------------------------------------------------------
        // X-axis (pitch)
        // -----------------------------------------------------------------------------
        X SinX = std::sin(_AngleInRadiensX);
        X CosX = std::cos(_AngleInRadiensX);

        // -----------------------------------------------------------------------------
        // Y-axis (yaw)
        // -----------------------------------------------------------------------------
        X SinY = std::sin(_AngleInRadiensY);
        X CosY = std::cos(_AngleInRadiensY);

        // -----------------------------------------------------------------------------
        // Z-axis (roll)
        // -----------------------------------------------------------------------------
        X SinZ = std::sin(_AngleInRadiensZ);
        X CosZ = std::cos(_AngleInRadiensZ);

        m_V[A11] =  CosY * CosZ; m_V[A12] =  SinX * SinY * CosZ + CosX * SinZ; m_V[A13] = -CosX * SinY * CosZ + SinX * SinZ;
        m_V[A21] = -CosY * SinZ; m_V[A22] = -SinX * SinY * SinZ + CosX * CosZ; m_V[A23] =  CosX * SinY * SinZ + SinX * CosZ;
        m_V[A31] =  SinY;        m_V[A32] = -SinX * CosY;                      m_V[A33] =  CosX * CosY;

        return *this;
    }

    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRotationX(X _AngleInRadiens)
    {
        SetIdentity();

        return InjectRotationX(_AngleInRadiens);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRotationY(X _AngleInRadiens)
    {
        SetIdentity();

        return InjectRotationY(_AngleInRadiens);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRotationZ(X _AngleInRadiens)
    {
        SetIdentity();

        return InjectRotationZ(_AngleInRadiens);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRotation(X _AngleInRadiensX, X _AngleInRadiensY, X _AngleInRadiensZ)
    {
        SetIdentity();

        return InjectRotation(_AngleInRadiensX, _AngleInRadiensY, _AngleInRadiensZ);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetRotation(CMatrix3x3<T>& _rRotationMatrix) const
    {
        _rRotationMatrix.Set(
            m_V[A11], m_V[A12], m_V[A13],
            m_V[A21], m_V[A22], m_V[A23],
            m_V[A31], m_V[A32], m_V[A33]
        );
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CMatrix4x4<T>::GetRotation(CVector3<T>& _rRotation)
    {
        _rRotation[0] = atan2(m_V[A32], m_V[A33]);
        _rRotation[1] = atan2(-m_V[A31], sqrt(m_V[A32] * m_V[A32] + m_V[A33] * m_V[A33]));
        _rRotation[2] = atan2(m_V[A21], m_V[A11]);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::LookAt(const Float3& _rEye, const Float3& _rTarget, const Float3& _rUp)
    {
        Base::Float3 LookDirection = (_rEye - _rTarget).Normalize();
        Base::Float3 Right         = LookDirection.CrossProduct(_rUp).Normalize();
        Base::Float3 Up            = LookDirection.CrossProduct(Right);
        
        m_V[A11] =          Right[0]; m_V[A12] =          Right[1]; m_V[A13] =           Right[2]; m_V[A14] =         -(Right.DotProduct(_rEye));
        m_V[A21] =             Up[0]; m_V[A22] =             Up[1]; m_V[A23] =              Up[2]; m_V[A24] =            -(Up.DotProduct(_rEye));
        m_V[A31] =  LookDirection[0]; m_V[A32] =  LookDirection[1]; m_V[A33] =   LookDirection[2]; m_V[A34] = -(LookDirection.DotProduct(_rEye));
        m_V[A41] =              X(0); m_V[A42] =              X(0); m_V[A43] =               X(0); m_V[A44] =                               X(1);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRHPerspective(X _Width, X _Height, X _Near, X _Far)
    {
        m_V[A11] = X(2) * _Near / _Width; m_V[A12] =                   X(0); m_V[A13] =                  X(0); m_V[A14] = X( 0);
        m_V[A21] =                  X(0); m_V[A22] = X(2) * _Near / _Height; m_V[A23] =                  X(0); m_V[A24] = X( 0);
        m_V[A31] =                  X(0); m_V[A32] =                   X(0); m_V[A33] = _Far / (_Near - _Far); m_V[A34] = _Far * _Near / (_Near - _Far);
        m_V[A41] =                  X(0); m_V[A42] =                   X(0); m_V[A43] =                 X(-1); m_V[A44] = X( 0);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRHPerspective(X _Left, X _Right, X _Bottom, X _Top, X _Near, X _Far)
    {
        m_V[A11] = X(2) * _Near / (_Right - _Left); m_V[A12] =                            X(0); m_V[A13] =                  X(0); m_V[A14] = X( 0);
        m_V[A21] =                            X(0); m_V[A22] = X(2) * _Near / (_Top - _Bottom); m_V[A23] =                  X(0); m_V[A24] = X( 0);
        m_V[A31] =                            X(0); m_V[A32] =                            X(0); m_V[A33] = _Far / (_Near - _Far); m_V[A34] = _Far * _Near / (_Near - _Far);
        m_V[A41] =                            X(0); m_V[A42] =                            X(0); m_V[A43] =                 X(-1); m_V[A44] = X( 0);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRHPerspective(X _Near, X _Far, Base::CMatrix3x3<X> _CameraMatrix)
    {
        float FX = _CameraMatrix[0][0];  //< Focal length in x-axis
        float FY = _CameraMatrix[1][1];  //< Focal length in y-axis
        float CX = _CameraMatrix[0][2];  //< Camera primary point x
        float CY = _CameraMatrix[1][2];  //< Camera primary point y

        m_V[A11] = FX / CX; m_V[A12] =    X(0); m_V[A13] =                  X(0); m_V[A14] = X( 0);
        m_V[A21] =    X(0); m_V[A22] = FY / CY; m_V[A23] =                  X(0); m_V[A24] = X( 0);
        m_V[A31] =    X(0); m_V[A32] =    X(0); m_V[A33] = _Far / (_Near - _Far); m_V[A34] = _Far * _Near / (_Near - _Far);
        m_V[A41] =    X(0); m_V[A42] =    X(0); m_V[A43] =                 X(-1); m_V[A44] = X( 0);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRHFieldOfView(X _FOVY, X _Aspect, X _Near, X _Far)
    {
        const X Radian = Base::DegreesToRadians(_FOVY) / X(2);
        const X ScaleY = Base::Cos(Radian) / Base::Sin(Radian);
        const X ScaleX = ScaleY / _Aspect;
        
        m_V[A11] = ScaleX; m_V[A12] =   X(0); m_V[A13] =                  X(0); m_V[A14] = X( 0);
        m_V[A21] = X(0)  ; m_V[A22] = ScaleY; m_V[A23] =                  X(0); m_V[A24] = X( 0);
        m_V[A31] = X(0)  ; m_V[A32] =   X(0); m_V[A33] = _Far / (_Near - _Far); m_V[A34] = _Far * _Near / (_Near - _Far);
        m_V[A41] = X(0)  ; m_V[A42] =   X(0); m_V[A43] =                 X(-1); m_V[A44] = X( 0);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRHOrthographic(X _Width, X _Height, X _Near, X _Far)
    {
        m_V[A11] = X(2) / _Width; m_V[A12] =           X(0); m_V[A13] =                  X(0); m_V[A14] = X(0);
        m_V[A21] =          X(0); m_V[A22] = X(2) / _Height; m_V[A23] =                  X(0); m_V[A24] = X(0);
        m_V[A31] =          X(0); m_V[A32] =           X(0); m_V[A33] = X(1) / (_Near - _Far); m_V[A34] = _Near / (_Near - _Far);
        m_V[A41] =          X(0); m_V[A42] =           X(0); m_V[A43] =                  X(0); m_V[A44] = X(1);

        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline typename CMatrix4x4<T>::CThis& CMatrix4x4<T>::SetRHOrthographic(X _Left, X _Right, X _Bottom, X _Top, X _Near, X _Far)
    {
        m_V[A11] = X(2) / (_Right - _Left); m_V[A12] =                    X(0); m_V[A13] =                  X(0); m_V[A14] = (_Left + _Right) / (_Left - _Right);
        m_V[A21] =                    X(0); m_V[A22] = X(2) / (_Top - _Bottom); m_V[A23] =                  X(0); m_V[A24] = (_Top + _Bottom) / (_Bottom - _Top);
        m_V[A31] =                    X(0); m_V[A32] =                    X(0); m_V[A33] = X(1) / (_Near - _Far); m_V[A34] = _Near / (_Near - _Far);
        m_V[A41] =                    X(0); m_V[A42] =                    X(0); m_V[A43] =                  X(0); m_V[A44] = X(1);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CMatrix4x4<T>::IsEqual(const CThis& _rRight, X _Epsilon) const
    {
        return MATH::IsEqual(m_V[A11], _rRight.m_V[A11], _Epsilon)
            && MATH::IsEqual(m_V[A12], _rRight.m_V[A12], _Epsilon)
            && MATH::IsEqual(m_V[A13], _rRight.m_V[A13], _Epsilon)
            && MATH::IsEqual(m_V[A14], _rRight.m_V[A14], _Epsilon)
            && MATH::IsEqual(m_V[A21], _rRight.m_V[A21], _Epsilon)
            && MATH::IsEqual(m_V[A22], _rRight.m_V[A22], _Epsilon)
            && MATH::IsEqual(m_V[A23], _rRight.m_V[A23], _Epsilon)
            && MATH::IsEqual(m_V[A24], _rRight.m_V[A24], _Epsilon)
            && MATH::IsEqual(m_V[A31], _rRight.m_V[A31], _Epsilon)
            && MATH::IsEqual(m_V[A32], _rRight.m_V[A32], _Epsilon)
            && MATH::IsEqual(m_V[A33], _rRight.m_V[A33], _Epsilon)
            && MATH::IsEqual(m_V[A34], _rRight.m_V[A34], _Epsilon)
            && MATH::IsEqual(m_V[A41], _rRight.m_V[A41], _Epsilon)
            && MATH::IsEqual(m_V[A42], _rRight.m_V[A42], _Epsilon)
            && MATH::IsEqual(m_V[A43], _rRight.m_V[A43], _Epsilon)
            && MATH::IsEqual(m_V[A44], _rRight.m_V[A44], _Epsilon);
    }
} // namespace MATH