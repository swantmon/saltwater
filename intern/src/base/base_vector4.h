
#pragma once

#include "base/base_defines.h"
#include "base/base_math_operations.h"

#include <assert.h>

namespace MATH
{
    template <typename T>
    class CVector3;
    
    template <typename T>
    class CMatrix4x4;
} // namespace MATH

namespace MATH
{
    template<typename T>
    class CVector4
    {
    public:

        static const int s_MaxNumberOfElements = 4;

    public:

        typedef CVector4<T>  CThis;
        typedef T            X;
        typedef X&           XRef;
        typedef const X&     XConstRef;
        typedef X*           XPtr;
        typedef const X*     XConstPtr;
        
        typedef CVector3<T> CThis3;
        
    public:

        static const CVector4<T> s_Zero;
        static const CVector4<T> s_One;
        static const CVector4<T> s_AxisX;
        static const CVector4<T> s_AxisY;
        static const CVector4<T> s_AxisZ;
        static const CVector4<T> s_AxisW;

    public:

        CVector4();
        CVector4(T _Value);
        CVector4(T _Value0, T _Value1);
        CVector4(T _Value0, T _Value1, T _Value2);
        CVector4(T _Value0, T _Value1, T _Value2, T _Value3);
        CVector4(const CThis3& _rValue0, T _Value1);
        CVector4(const CThis& _rCpy);
        
    public:
        
        inline void Set(T _Value);
        inline void Set(T _Value0, T _Value1);
        inline void Set(T _Value0, T _Value1, T _Value2);
        inline void Set(T _Value0, T _Value1, T _Value2, T _Value3);
        inline void Set(const CThis3& _rValue0, T _Value1);
        
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

        inline CThis operator * (const CMatrix4x4<T>& _rMatrix) const;

    public:

        inline CThis& operator -= (const CThis& _rRight);
        inline CThis& operator += (const CThis& _rRight);
        inline CThis& operator *= (const CThis& _rRight);
        inline CThis& operator /= (const CThis& _rRight);

        inline CThis& operator *= (const T _Scalar);
        inline CThis& operator /= (const T _Scalar);

    public:

        inline T Length() const;

        inline CThis Normalize() const;

    private:

        T m_V[s_MaxNumberOfElements];

    };
} // namespace MATH

namespace MATH
{
    typedef CVector4<unsigned char> Byte4;
    typedef CVector4<int>           Int4;
    typedef CVector4<unsigned int>  UInt4;
    typedef CVector4<float>         Float4;
    typedef CVector4<double>        Double4;
} // namespace MATH

namespace MATH
{
    template<typename T>
    const CVector4<T> CVector4<T>::s_Zero  = CVector4(T(0));

    template<typename T>
    const CVector4<T> CVector4<T>::s_One   = CVector4(T(1), T(1), T(1), T(1));

    template<typename T>
    const CVector4<T> CVector4<T>::s_AxisX = CVector4(T(1), T(0), T(0), T(0));

    template<typename T>
    const CVector4<T> CVector4<T>::s_AxisY = CVector4(T(0), T(1), T(0), T(0));

    template<typename T>
    const CVector4<T> CVector4<T>::s_AxisZ = CVector4(T(0), T(0), T(1), T(0));

    template<typename T>
    const CVector4<T> CVector4<T>::s_AxisW = CVector4(T(0), T(0), T(0), T(1));

} // namespace MATH

namespace MATH
{
    template<typename T>
    CVector4<T>::CVector4()
    {
        Set(T(0));
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector4<T>::CVector4(T _Value)
    {
        Set(_Value);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector4<T>::CVector4(T _Value0, T _Value1)
    {
        Set(_Value0, _Value1);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector4<T>::CVector4(T _Value0, T _Value1, T _Value2)
    {
        Set(_Value0, _Value1, _Value2);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector4<T>::CVector4(T _Value0, T _Value1, T _Value2, T _Value3)
    {
        Set(_Value0, _Value1, _Value2, _Value3);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CVector4<T>::CVector4(const CThis3& _rValue0, T _Value1)
    {
        Set(_rValue0, _Value1);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector4<T>::CVector4(const CThis& _rCpy)
    {
        m_V[0] = _rCpy[0];
        m_V[1] = _rCpy[1];
        m_V[2] = _rCpy[2];
        m_V[3] = _rCpy[3];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CVector4<T>::Set(T _Value)
    {
        m_V[0] = _Value;
        m_V[1] = _Value;
        m_V[2] = _Value;
        m_V[3] = _Value;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CVector4<T>::Set(T _Value0, T _Value1)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = T(0);
        m_V[3] = T(0);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CVector4<T>::Set(T _Value0, T _Value1, T _Value2)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = _Value2;
        m_V[3] = T(0);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CVector4<T>::Set(T _Value0, T _Value1, T _Value2, T _Value3)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = _Value2;
        m_V[3] = _Value3;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CVector4<T>::Set(const CThis3& _rValue0, T _Value1)
    {
        m_V[0] = _rValue0[0];
        m_V[1] = _rValue0[1];
        m_V[2] = _rValue0[2];
        m_V[3] = _Value1;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CVector4<T>::SetZero()
    {
        m_V[0] = X(0);
        m_V[1] = X(0);
        m_V[2] = X(0);
        m_V[3] = X(0);
    }
    
    // -----------------------------------------------------------------------------
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline bool CVector4<T>::IsEqual(const CThis& _rVector, X _Epsilon) const
    {
        return Base::IsEqual(m_V[0], _rVector[0], _Epsilon) && Base::IsEqual(m_V[1], _rVector[1], _Epsilon) && Base::IsEqual(m_V[2], _rVector[2], _Epsilon) && Base::IsEqual(m_V[3], _rVector[3], _Epsilon);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline bool CVector4<T>::IsEqual(const CThis& _rVector, const CThis& _rEpsilon) const
    {
        return Base::IsEqual(m_V[0], _rVector[0], _rEpsilon[0]) && Base::IsEqual(m_V[1], _rVector[1], _rEpsilon[1]) && Base::IsEqual(m_V[2], _rVector[2], _rEpsilon[2]) && Base::IsEqual(m_V[3], _rVector[3], _rEpsilon[3]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator = (CThis const& _rRight)
    {
        m_V[0] = _rRight[0];
        m_V[1] = _rRight[1];
        m_V[2] = _rRight[2];
        m_V[3] = _rRight[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline T& CVector4<T>::operator [] (const int _Index)
    {
        return m_V[_Index];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline const T& CVector4<T>::operator [] (const int _Index) const
    {
        return m_V[_Index];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline bool CVector4<T>::operator == (const CThis& _rRight) const
    {
        bool IsEqual;

        IsEqual = m_V[0] == _rRight.m_V[0] && m_V[1] == _rRight.m_V[1] && m_V[2] == _rRight.m_V[2] && m_V[3] == _rRight.m_V[3];

        return IsEqual;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline bool CVector4<T>::operator != (const CThis& _rRight) const
    {
        bool IsEqual;

        IsEqual = m_V[0] != _rRight.m_V[0] || m_V[1] != _rRight.m_V[1] || m_V[2] != _rRight.m_V[2] || m_V[3] != _rRight.m_V[3];

        return IsEqual;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator - (const CThis& _rRight) const
    {
        CThis MinThis;

        MinThis.m_V[0] = this->m_V[0] - _rRight.m_V[0];
        MinThis.m_V[1] = this->m_V[1] - _rRight.m_V[1];
        MinThis.m_V[2] = this->m_V[2] - _rRight.m_V[2];
        MinThis.m_V[3] = this->m_V[3] - _rRight.m_V[3];

        return MinThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator + (const CThis& _rRight) const
    {
        CThis AddThis;

        AddThis.m_V[0] = this->m_V[0] + _rRight.m_V[0];
        AddThis.m_V[1] = this->m_V[1] + _rRight.m_V[1];
        AddThis.m_V[2] = this->m_V[2] + _rRight.m_V[2];
        AddThis.m_V[3] = this->m_V[3] + _rRight.m_V[3];

        return AddThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator * (const CThis& _rRight) const
    {
        CThis MulThis;

        MulThis.m_V[0] = this->m_V[0] * _rRight.m_V[0];
        MulThis.m_V[1] = this->m_V[1] * _rRight.m_V[1];
        MulThis.m_V[2] = this->m_V[2] * _rRight.m_V[2];
        MulThis.m_V[3] = this->m_V[3] * _rRight.m_V[3];

        return MulThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator / (const CThis& _rRight) const
    {
        CThis DevThis;

        DevThis.m_V[0] = this->m_V[0] / _rRight.m_V[0];
        DevThis.m_V[1] = this->m_V[1] / _rRight.m_V[1];
        DevThis.m_V[2] = this->m_V[2] / _rRight.m_V[2];
        DevThis.m_V[3] = this->m_V[3] / _rRight.m_V[3];

        return DevThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator * (const T _Scalar) const
    {
        CThis MulThis;

        MulThis.m_V[0] = this->m_V[0] * _Scalar;
        MulThis.m_V[1] = this->m_V[1] * _Scalar;
        MulThis.m_V[2] = this->m_V[2] * _Scalar;
        MulThis.m_V[3] = this->m_V[3] * _Scalar;

        return MulThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator / (const T _Scalar) const
    {
        CThis DevThis;

        DevThis.m_V[0] = this->m_V[0] / _Scalar;
        DevThis.m_V[1] = this->m_V[1] / _Scalar;
        DevThis.m_V[2] = this->m_V[2] / _Scalar;
        DevThis.m_V[3] = this->m_V[3] / _Scalar;

        return DevThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::operator * (const CMatrix4x4<T>& _rMatrix) const
    {
        CThis MulThis;

        MulThis.m_V[0] = this->m_V[0] * _rMatrix[0][0] + this->m_V[1] * _rMatrix[1][0] + this->m_V[2] * _rMatrix[2][0] + this->m_V[3] * _rMatrix[3][0];
        MulThis.m_V[1] = this->m_V[0] * _rMatrix[0][1] + this->m_V[1] * _rMatrix[1][1] + this->m_V[2] * _rMatrix[2][1] + this->m_V[3] * _rMatrix[3][1];
        MulThis.m_V[2] = this->m_V[0] * _rMatrix[0][2] + this->m_V[1] * _rMatrix[1][2] + this->m_V[2] * _rMatrix[2][2] + this->m_V[3] * _rMatrix[3][2];
        MulThis.m_V[3] = this->m_V[0] * _rMatrix[0][3] + this->m_V[1] * _rMatrix[1][3] + this->m_V[2] * _rMatrix[2][3] + this->m_V[3] * _rMatrix[3][3];

        return MulThis;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator -= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] - _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] - _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] - _rRight.m_V[2];
        this->m_V[3] = this->m_V[3] - _rRight.m_V[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator += (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] + _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] + _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] + _rRight.m_V[2];
        this->m_V[3] = this->m_V[3] + _rRight.m_V[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator *= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] * _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] * _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] * _rRight.m_V[2];
        this->m_V[3] = this->m_V[3] * _rRight.m_V[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator /= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] / _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] / _rRight.m_V[1];
        this->m_V[2] = this->m_V[2] / _rRight.m_V[2];
        this->m_V[3] = this->m_V[3] / _rRight.m_V[3];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator *= (const T _Scalar)
    {
        this->m_V[0] = this->m_V[0] * _Scalar;
        this->m_V[1] = this->m_V[1] * _Scalar;
        this->m_V[2] = this->m_V[2] * _Scalar;
        this->m_V[3] = this->m_V[3] * _Scalar;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis& CVector4<T>::operator /= (const T _Scalar)
    {
        this->m_V[0] = this->m_V[0] / _Scalar;
        this->m_V[1] = this->m_V[1] / _Scalar;
        this->m_V[2] = this->m_V[2] / _Scalar;
        this->m_V[3] = this->m_V[3] / _Scalar;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline T CVector4<T>::Length() const
    {
        return sqrt( (this->m_V[0] * this->m_V[0]) + (this->m_V[1] * this->m_V[1]) + (this->m_V[2] * this->m_V[2]) + (this->m_V[3] * this->m_V[3]) );
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector4<T>::CThis CVector4<T>::Normalize() const
    {
        CThis Normalize;

        float Factor;

        T Length;

        Length = this->Length();

        assert(Length != 0);

        Factor = 1.0f / Length;

        Normalize.m_V[0] = this->m_V[0] * Factor;
        Normalize.m_V[1] = this->m_V[1] * Factor;
        Normalize.m_V[2] = this->m_V[2] * Factor;
        Normalize.m_V[3] = this->m_V[3] * Factor;

        return Normalize;
    }

} // namespace MATH
