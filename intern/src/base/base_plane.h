
#pragma once

#include "base/base_math_constants.h"
#include "base/base_math_operations.h"

#include "glm.hpp"

namespace MATH
{
    template <typename T>
    class CPlane
    {
    public:

        static const CPlane<T> s_AxisX;
        static const CPlane<T> s_AxisY;
        static const CPlane<T> s_AxisZ;

    public:

        typedef CPlane<T>     CThis;
        typedef T             X;
        typedef T*            XPtr;
        typedef const T*      XConstPtr;
        typedef T&            XRef;
        typedef const T&      XConstRef;
        typedef glm::tvec3<T> CVector;

    public:

        static const unsigned int s_NumberOfComponents = 4;

    public:

        inline CPlane();
        inline CPlane(const CThis& _rPlane);
        inline CPlane(X _NormalX, X _NormalY, X _NormalZ, X _Distance);
        inline CPlane(const CVector& _rNormal, const CVector& _rVector);
        inline CPlane(const CVector& _rVector0, const CVector& _rVector1, const CVector& _rVector2);
        inline CPlane(const glm::tvec4<X>& _rVector);

    public:

        inline CThis& operator = (const CThis& _rPlane);
        inline CThis& operator = (const glm::tvec4<X>& _rVector);

    public:

        inline XRef operator [] (unsigned int _Index);
        inline XConstRef operator [] (unsigned int _Index) const;

    public:

        inline bool operator == (const CThis& _rPlane) const;
        inline bool operator != (const CThis& _rPlane) const;

    public:

        inline CThis& SetZero();

        inline CThis& Set(const CThis& _rPlane);
        inline CThis& Set(X _NormalX, X _NormalY, X _NormalZ, X _Distance);
        inline CThis& Set(const CVector& _rNormal, const CVector& _rVector);
        inline CThis& Set(const CVector& _rVector0, const CVector& _rVector1, const CVector& _rVector2);
        inline CThis& Set(const glm::tvec4<X>& _rVector);

    public:

        inline bool IsEqual(const CThis& _rPlane, X _Epsilon) const;
        inline bool IsEqual(const CThis& _rPlane, const glm::tvec4<X>& _rEpsilon) const;

    public:

        inline bool IntersectsRay(const CVector& _rOrigin, const CVector& _rDirection, CVector& _rPosition, X& _rLambda) const;

    public:

        inline X GetDistance(const CVector& _rPoint) const;

    private:

        glm::tvec4<X> m_P;
    };
} // namespace MATH

namespace MATH
{
    template <typename T>
    const CPlane<T> CPlane<T>::s_AxisX = CPlane(T(1), T(0), T(0), T(0));
    template <typename T>
    const CPlane<T> CPlane<T>::s_AxisY = CPlane(T(0), T(1), T(0), T(0));
    template <typename T>
    const CPlane<T> CPlane<T>::s_AxisZ = CPlane(T(0), T(0), T(1), T(0));
} // namespace MATH

namespace MATH
{
    template <typename T>
    inline CPlane<T>::CPlane()
    {
        SetZero();
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline CPlane<T>::CPlane(const CThis& _rPlane)
    {
        Set(_rPlane);
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline CPlane<T>::CPlane(X _NormalX, X _NormalY, X _NormalZ, X _Distance)
    {
        Set(_NormalX, _NormalY, _NormalZ, _Distance);
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline CPlane<T>::CPlane(const CVector& _rNormal, const CVector& _rVector)
    {
        Set(_rNormal, _rVector);
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline CPlane<T>::CPlane(const CVector& _rVector0, const CVector& _rVector1, const CVector& _rVector2)
    {
        Set(_rVector0, _rVector1, _rVector2);
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline CPlane<T>::CPlane(const glm::tvec4<X>& _rVector)
    {
        Set(_rVector);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::operator = (const CThis& _rPlane)
    {
        return Set(_rPlane);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::operator = (const glm::tvec4<X>& _rVector)
    {
        return Set(_rVector);
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::XRef CPlane<T>::operator [] (unsigned int _Index)
    {
        return m_P[_Index];
    }

    // -----------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::XConstRef CPlane<T>::operator [] (unsigned int _Index) const
    {
        return m_P[_Index];
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline bool CPlane<T>::operator == (const CThis& _rPlane) const
    {
        return m_P == _rPlane.m_P;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline bool CPlane<T>::operator != (const CThis& _rPlane) const
    {
        return m_P != _rPlane.m_P;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::SetZero()
    {
        m_P = glm::tvec4<T>(0.0f);

        return *this;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::Set(const CThis& _rPlane)
    {
        m_P = _rPlane.m_P;

        return *this;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::Set(X _NormalX, X _NormalY, X _NormalZ, X _Distance)
    {
        m_P = glm::tvec4<T>(_NormalX, _NormalY, _NormalZ, _Distance);

        return *this;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::Set(const CVector& _rNormal, const CVector& _rVector)
    {
        m_P.Set(_rNormal[0], _rNormal[1], _rNormal[2], -glm::dot(_rNormal, _rVector));

        return *this;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::Set(const CVector& _rVector0, const CVector& _rVector1, const CVector& _rVector2)
    {
        // ------------------------------------------------------------------------
        // Constructs the plane for an anti-clockwise winding of the points.
        // ------------------------------------------------------------------------
        CVector Normal = glm::normalize(glm::cross((_rVector1 - _rVector0), (_rVector2 - _rVector0)));

        m_P.Set(Normal[0], Normal[1], Normal[2], -glm::dot(Normal, _rVector0));

        return *this;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::CThis& CPlane<T>::Set(const glm::tvec4<X>& _rVector)
    {
        m_P = _rVector;

        return *this;
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CPlane<T>::IsEqual(const CThis& _rPlane, X _Epsilon) const
    {
        return glm::epsilonEqual(m_P, _rPlane.m_P, _Epsilon);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CPlane<T>::IsEqual(const CThis& _rPlane, const glm::tvec4<X>& _rEpsilon) const
    {
        return glm::epsilonEqual(m_P, _rPlane.m_P, _rEpsilon);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CPlane<T>::IntersectsRay(const CVector& _rOrigin, const CVector& _rDirection, CVector& _rHitPoint, X& _rLambda) const
    {
        X Cosine;
        X Numerator;
        X Lambda;

        // ------------------------------------------------------------------------
        // Check if the ray is perpendicular to the normal.
        // ------------------------------------------------------------------------
        Cosine = _rDirection[0] * m_P[0] + _rDirection[1] * m_P[1] + _rDirection[2] * m_P[2];

        if (MATH::IsEqual(Cosine, X(0), MATH::SConstants<X>::s_Epsilon))
        {
            return false;
        }

        // ------------------------------------------------------------------------
        // Calculate the hit point.
        // ------------------------------------------------------------------------
        Numerator = _rOrigin[0] * m_P[0] + _rOrigin[1] * m_P[1] + _rOrigin[2] * m_P[2] + m_P[3];

        Lambda = -Numerator / Cosine;

        // ------------------------------------------------------------------------
        // Check if the intersection is before the origin.
        // ------------------------------------------------------------------------
        if (Lambda < X(0))
        {
            return false;
        }

        _rHitPoint = _rOrigin + (_rDirection * Lambda); _rLambda = Lambda;

        return true;
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CPlane<T>::X CPlane<T>::GetDistance(const CVector& _rPoint) const
    {
        return _rPoint[0] * m_P[0] + _rPoint[1] * m_P[1] + _rPoint[2] * m_P[2] + m_P[3];
    }
} // namespace MATH