//
//  base_quaternion.h
//  base
//
//  Created by Tobias Schwandt on 10/08/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"
#include "base/base_math_operations.h"

#include <assert.h>
#include <math.h>

namespace MATH
{
    template <typename T>
    class CVector3;

    template <typename T>
    class CVector4;

    template <typename T>
    class CMatrix3x3;

    template <typename T>
    class CMatrix4x4;
} // namespace MATH

namespace MATH
{
    template <typename T>
    class CQuaternion
    {
    public:

        typedef CQuaternion<T> CThis;
        typedef T              X;
        typedef T*             XPtr;
        typedef const T*       XConstPtr;
        typedef T&             XRef;
        typedef const T&       XConstRef;

    public:

        static const unsigned int s_NumberOfComponents = 4;

    public:

        inline CQuaternion();
        inline CQuaternion(const CThis& _rQuaternion);
        inline CQuaternion(X _Value);
        inline CQuaternion(X _Value0, X _Value1, X _Value2, X _Value3);

    public:

        inline CThis& SetIdentity();

        inline CThis& Set(X _Value);
        inline CThis& Set(X _Value0, X _Value1, X _Value2, X _Value3);
        inline CThis& Set(const MATH::CVector3<T>& _rVector, X _AngleInRadians);
        inline CThis& Set(const MATH::CVector4<T>& _rVector, X _AngleInRadians);
        inline CThis& Set(const MATH::CMatrix3x3<T>& _rMatrix);
        inline CThis& Set(const MATH::CMatrix4x4<T>& _rMatrix);

    public:

        inline CThis& NLerp(const CThis& _rOther, X _Weight);
        inline CThis GetNLerp(const CThis& _rOther, X _Weight) const;

        inline CThis& SLerp(const CThis& _rOther, X _Weight);
        inline CThis GetSLerp(const CThis& _rOther, X _Weight) const;
        
    public:
        
        inline CMatrix3x3<T> GetRHRotationMatrix() const;

    public:

        inline CThis& operator = (const CThis& _rRight);

    public:

        inline XRef operator [] (unsigned int _Index);
        inline XConstRef operator [] (unsigned int _Index) const;
        
    public:
        
        bool IsEqual(const CThis& _rRight, X _Epsilon) const;

    private:

        X m_V[s_NumberOfComponents];
    };
}

namespace MATH
{
    typedef CQuaternion<float> FloatQuaternion;
} // namespace MATH

namespace MATH
{
    template <typename T>
    inline CQuaternion<T>::CQuaternion()
    {
        SetIdentity();
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline CQuaternion<T>::CQuaternion(const CThis& _rQuaternion)
    {
        Set(_rQuaternion[0], _rQuaternion[1], _rQuaternion[2], _rQuaternion[3]);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline CQuaternion<T>::CQuaternion(X _Value)
    {
        Set(_Value);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline CQuaternion<T>::CQuaternion(X _Value0, X _Value1, X _Value2, X _Value3)
    {
        Set(_Value0, _Value1, _Value2, _Value3);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::SetIdentity()
    {
        Set(X(0), X(0), X(0), X(1));
        
        return *this;
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::Set(X _Value)
    {
        Set(_Value, _Value, _Value, _Value);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::Set(X _Value0, X _Value1, X _Value2, X _Value3)
    {
        m_V[0] = _Value0;
        m_V[1] = _Value1;
        m_V[2] = _Value2;
        m_V[3] = _Value3;
        
        return *this;
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::Set(const MATH::CVector3<T>& _rVector, X _AngleInRadians)
    {
        float HalfAngle = _AngleInRadians / 2.0f;
        float SinAngle  = sin(HalfAngle);
        float CosAngle  = cos(HalfAngle);
        
        MATH::CVector3<T> Normal = _rVector.Normalize();
        
        m_V[0] = Normal[0] * SinAngle;
        m_V[1] = Normal[1] * SinAngle;
        m_V[2] = Normal[2] * SinAngle;
        m_V[3] = CosAngle;
        
        return *this;
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::Set(const MATH::CVector4<T>& _rVector, X _AngleInRadians)
    {
        float HalfAngle = _AngleInRadians / 2.0f;
        float SinAngle  = sin(HalfAngle);
        float CosAngle  = cos(HalfAngle);
        
        MATH::CVector4<T> Normal = _rVector.Normalize();
        
        m_V[0] = Normal[0] * SinAngle;
        m_V[1] = Normal[1] * SinAngle;
        m_V[2] = Normal[2] * SinAngle;
        m_V[3] = CosAngle;
        
        return *this;
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::Set(const MATH::CMatrix3x3<T>& _rMatrix)
    {

    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::Set(const MATH::CMatrix4x4<T>& _rMatrix)
    {

    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::NLerp(const CThis& _rOther, X _Weight)
    {

    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis CQuaternion<T>::GetNLerp(const CThis& _rOther, X _Weight) const
    {

    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::SLerp(const CThis& _rOther, X _Weight)
    {
        float Omega = acos(saturate(m_V[0] * _rOther.m_V[0] +
                                    m_V[1] * _rOther.m_V[1] +
                                    m_V[2] * _rOther.m_V[2] +
                                    m_V[3] * _rOther.m_V[3], -1.0f,1.0f));
        if (fabs(Omega) < 1e-10)
        {
            Omega = 1e-10;
        }
        
        double SinOmega       = sin(Omega);
        double LeftDeviation  = sin((1.0f - _Weight) * Omega) / SinOmega;
        double RightDeviation = sin(_Weight * Omega) / SinOmega;
        
        return Set(m_V[0] * LeftDeviation + _rOther.m_V[0] * RightDeviation,
                   m_V[1] * LeftDeviation + _rOther.m_V[1] * RightDeviation,
                   m_V[2] * LeftDeviation + _rOther.m_V[2] * RightDeviation,
                   m_V[3] * LeftDeviation + _rOther.m_V[3] * RightDeviation);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis CQuaternion<T>::GetSLerp(const CThis& _rOther, X _Weight) const
    {
        float Omega = acos(MATH::Clamp(m_V[0] * _rOther.m_V[0] +
                                       m_V[1] * _rOther.m_V[1] +
                                       m_V[2] * _rOther.m_V[2] +
                                       m_V[3] * _rOther.m_V[3], -1.0f, 1.0f));
        if (fabs(Omega) < 1e-10)
        {
            Omega = 1e-10;
        }
        
        double SinOmega       = sin(Omega);
        double LeftDeviation  = sin((1.0f - _Weight) * Omega) / SinOmega;
        double RightDeviation = sin(_Weight * Omega) / SinOmega;
        
        return CThis(m_V[0] * LeftDeviation + _rOther.m_V[0] * RightDeviation,
                     m_V[1] * LeftDeviation + _rOther.m_V[1] * RightDeviation,
                     m_V[2] * LeftDeviation + _rOther.m_V[2] * RightDeviation,
                     m_V[3] * LeftDeviation + _rOther.m_V[3] * RightDeviation);
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    inline CMatrix3x3<T> CQuaternion<T>::GetRHRotationMatrix() const
    {
        const float M[9] =
        {
            1.0f - 2.0f * m_V[1] * m_V[1] - 2.0f * m_V[2] * m_V[2], 2.0f        * m_V[0] * m_V[1] - 2.0f * m_V[2] * m_V[3], 2.0f        * m_V[0] * m_V[2] + 2.0f * m_V[1] * m_V[3],
            2.0f        * m_V[0] * m_V[1] + 2.0f * m_V[2] * m_V[3], 1.0f - 2.0f * m_V[0] * m_V[0] - 2.0f * m_V[2] * m_V[2], 2.0f        * m_V[1] * m_V[2] - 2.0f * m_V[0] * m_V[3],
            2.0f        * m_V[0] * m_V[2] - 2.0f * m_V[1] * m_V[3], 2.0f        * m_V[1] * m_V[2] + 2.0f * m_V[0] * m_V[3], 1.0f - 2.0f * m_V[0] * m_V[0] - 2.0f * m_V[1] * m_V[1]
        };
        
        return CMatrix3x3<T>(M);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::CThis& CQuaternion<T>::operator = (const CThis& _rRight)
    {
        Set(_rRight[0], _rRight[1], _rRight[2], _rRight[3]);
        
        return *this;
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::XRef CQuaternion<T>::operator [] (unsigned int _Index)
    {
        return m_V[_Index];
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CQuaternion<T>::XConstRef CQuaternion<T>::operator [] (unsigned int _Index) const
    {
        return m_V[_Index];
    }
    
    // -----------------------------------------------------------------------------
    
    template <typename T>
    bool CQuaternion<T>::IsEqual(const CThis& _rRight, X _Epsilon) const
    {
        return MATH::IsEqual(m_V[0], _rRight[0], _Epsilon)
            && MATH::IsEqual(m_V[1], _rRight[1], _Epsilon)
            && MATH::IsEqual(m_V[2], _rRight[2], _Epsilon)
            && MATH::IsEqual(m_V[3], _rRight[3], _Epsilon);
    }
} // namespace MATH
