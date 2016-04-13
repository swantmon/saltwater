
#pragma once

#include "base/base_defines.h"

namespace MATH
{
    template<typename T>
    class CVector2
    {
    public:

        static const int s_MaxNumberOfElements = 2;

    public:

        typedef CVector2<T> CThis;

    public:

        CVector2();
        CVector2(T _Value);
        CVector2(T _Value0, T _Value1);
        CVector2(const CThis& _rCpy);

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
        
    public:
        
        inline CThis& operator -= (const CThis& _rRight);
        inline CThis& operator += (const CThis& _rRight);
        inline CThis& operator *= (const CThis& _rRight);
        inline CThis& operator /= (const CThis& _rRight);
        
        inline CThis& operator *= (const T _Scalar);
        inline CThis& operator /= (const T _Scalar);

    private:

        T m_V[s_MaxNumberOfElements];

    };
} // namespace MATH

namespace MATH
{
    typedef CVector2<unsigned char>  Byte2;
    typedef CVector2<int>            Int2;
    typedef CVector2<unsigned int>   UInt2;
    typedef CVector2<float>          Float2;
    typedef CVector2<double>         Double2;
    typedef CVector2<short>          Short2;
    typedef CVector2<unsigned short> UShort2;
} // namespace MATH

namespace MATH
{
    template<typename T>
    CVector2<T>::CVector2()
    {
        m_V[0] = 0;
        m_V[1] = 0;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector2<T>::CVector2(T _Value)
    {
        m_V[0] = _Value;
        m_V[1] = _Value;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector2<T>::CVector2(T _Value0, T _Value1)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CVector2<T>::CVector2(const CThis& _rCpy)
    {
        m_V[0] = _rCpy[0];
        m_V[1] = _rCpy[1];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator = (CThis const& _rRight)
    {
        m_V[0] = _rRight[0];
        m_V[1] = _rRight[1];

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline T& CVector2<T>::operator [] (const int _Index)
    {
        return m_V[_Index];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline const T& CVector2<T>::operator [] (const int _Index) const
    {
        return m_V[_Index];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline bool CVector2<T>::operator == (const CThis& _rRight) const
    {
        bool IsEqual;

        IsEqual = m_V[0] == _rRight.m_V[0] && m_V[1] == _rRight.m_V[1];

        return IsEqual;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline bool CVector2<T>::operator != (const CThis& _rRight) const
    {
        bool IsEqual;

        IsEqual = m_V[0] != _rRight.m_V[0] && m_V[1] != _rRight.m_V[1];

        return IsEqual;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis CVector2<T>::operator - (const CThis& _rRight) const
    {
        CThis MinThis;
        
        MinThis.m_V[0] = this->m_V[0] - _rRight.m_V[0];
        MinThis.m_V[1] = this->m_V[1] - _rRight.m_V[1];
        
        return MinThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis CVector2<T>::operator + (const CThis& _rRight) const
    {
        CThis AddThis;
        
        AddThis.m_V[0] = this->m_V[0] + _rRight.m_V[0];
        AddThis.m_V[1] = this->m_V[1] + _rRight.m_V[1];
        
        return AddThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis CVector2<T>::operator * (const CThis& _rRight) const
    {
        CThis MulThis;
        
        MulThis.m_V[0] = this->m_V[0] * _rRight.m_V[0];
        MulThis.m_V[1] = this->m_V[1] * _rRight.m_V[1];
        
        return MulThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis CVector2<T>::operator / (const CThis& _rRight) const
    {
        CThis DevThis;
        
        DevThis.m_V[0] = this->m_V[0] / _rRight.m_V[0];
        DevThis.m_V[1] = this->m_V[1] / _rRight.m_V[1];
        
        return DevThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis CVector2<T>::operator * (const T _Scalar) const
    {
        CThis MulThis;
        
        MulThis.m_V[0] = this->m_V[0] * _Scalar;
        MulThis.m_V[1] = this->m_V[1] * _Scalar;
        
        return MulThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis CVector2<T>::operator / (const T _Scalar) const
    {
        CThis DevThis;
        
        DevThis.m_V[0] = this->m_V[0] / _Scalar;
        DevThis.m_V[1] = this->m_V[1] / _Scalar;
        
        return DevThis;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator -= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] - _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] - _rRight.m_V[1];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator += (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] + _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] + _rRight.m_V[1];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator *= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] * _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] * _rRight.m_V[1];
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator /= (const CThis& _rRight)
    {
        this->m_V[0] = this->m_V[0] / _rRight.m_V[0];
        this->m_V[1] = this->m_V[1] / _rRight.m_V[1];;
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator *= (const T _Scalar)
    {
        this->m_V[0] = this->m_V[0] * _Scalar;
        this->m_V[1] = this->m_V[1] * _Scalar;
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline typename CVector2<T>::CThis& CVector2<T>::operator /= (const T _Scalar)
    {
        this->m_V[0] = this->m_V[0] / _Scalar;
        this->m_V[1] = this->m_V[1] / _Scalar;
        
        return *this;
    }
} // namespace MATH
