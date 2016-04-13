
#pragma once

#include "base/base_math_constants.h"
#include "base/base_math_operations.h"
#include "base/base_vector3.h"

#include <assert.h>

namespace MATH
{
    template <typename T>
    class CTriangle3
    {

        //
        //        +
        //      /   \
        //    /       \
        //  + - - - - - +
        //  

    public:

        typedef CTriangle3<T> CThis;
        typedef T             X;
        typedef const X       XConst;
        typedef X&            XRef;
        typedef const X&      XConstRef;
        typedef X*            XPtr;
        typedef const X*      XConstPtr;

        typedef CVector3<T>   CVector;

    public:

        static const unsigned int s_NumberOfVertices = 3;

    public:

        inline CTriangle3();
        inline CTriangle3(const CThis& _rTriangle);
        inline CTriangle3(const CVector& _rVertex0, const CVector& _rVertex1, const CVector& _rVertex2);

    public:

        inline CThis& operator = (const CThis& _rTriangle);

    public:

        inline CVector& operator [] (unsigned int _Index);
        inline const CVector& operator [] (unsigned int _Index) const;

    public:

        inline bool operator == (const CThis& _rTriangle) const;
        inline bool operator != (const CThis& _rTriangle) const;

    public:

        inline CThis& SetZero();

        inline CThis& Set(const CThis& _rTriangle);
        inline CThis& Set(const CVector& _rVertex0, const CVector& _rVertex1, const CVector& _rVertex2);

    public:

        inline bool IsEqual(const CThis& _rTriangle, X _Epsilon) const;
        inline bool IsEqual(const CThis& _rTriangle, const CVector& _rEpsilon) const;

    public:

        inline CVector GetNormal() const;

        inline X GetArea() const;

    public:

        inline bool IntersectsRay(const CVector& _rOrigin, const CVector& _rDirection, CVector& _rHitPoint, X& _rLambda) const;

    private:

        CVector m_Vertices[s_NumberOfVertices];
    };
} // namespace MATH

namespace MATH
{
    typedef CTriangle3<float>  Triangle3Float;
    typedef CTriangle3<double> Triangle3Double;
} // namespace MATH

namespace MATH
{
    template <typename T>
    inline CTriangle3<T>::CTriangle3()
    {
        SetZero();
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline CTriangle3<T>::CTriangle3(const CThis& _rTriangle)

    {
        Set(_rTriangle);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline CTriangle3<T>::CTriangle3(const CVector& _rVertex0, const CVector& _rVertex1, const CVector& _rVertex2)
    {
        Set(_rVertex0, _rVertex1, _rVertex2);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::CThis& CTriangle3<T>::operator = (const CThis& _rTriangle)
    {
        return Set(_rTriangle);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::CVector& CTriangle3<T>::operator [] (unsigned int _Index)
    {
        return const_cast<CVector&>(static_cast<const CThis&>(*this)[_Index]);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline const typename CTriangle3<T>::CVector& CTriangle3<T>::operator [] (unsigned int _Index) const
    {
        assert(_Index < s_NumberOfVertices);

        return m_Vertices[_Index];
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CTriangle3<T>::operator == (const CThis& _rTriangle) const
    {
        return (m_Vertices[0] == _rTriangle.m_Vertices[0]) && (m_Vertices[1] == _rTriangle.m_Vertices[1]) && (m_Vertices[2] == _rTriangle.m_Vertices[2]);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CTriangle3<T>::operator != (const CThis& _rTriangle) const
    {
        return (m_Vertices[0] != _rTriangle.m_Vertices[0]) || (m_Vertices[1] != _rTriangle.m_Vertices[1]) || (m_Vertices[2] != _rTriangle.m_Vertices[2]);
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::CThis& CTriangle3<T>::SetZero()
    {
        m_Vertices[0].SetZero();
        m_Vertices[1].SetZero();
        m_Vertices[2].SetZero();

        return *this;
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::CThis& CTriangle3<T>::Set(const CThis& _rTriangle)
    {
        m_Vertices[0].Set(_rTriangle.m_Vertices[0]);
        m_Vertices[1].Set(_rTriangle.m_Vertices[1]);
        m_Vertices[2].Set(_rTriangle.m_Vertices[2]);

        return *this;
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::CThis& CTriangle3<T>::Set(const CVector& _rVertex0, const CVector& _rVertex1, const CVector& _rVertex2)
    {
        m_Vertices[0].Set(_rVertex0);
        m_Vertices[1].Set(_rVertex1);
        m_Vertices[2].Set(_rVertex2);

        return *this;
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CTriangle3<T>::IsEqual(const CThis& _rTriangle, X _Epsilon) const
    {
        return (m_Vertices[0].IsEqual(_rTriangle.m_Vertices[0], _Epsilon)) && (m_Vertices[1].IsEqual(_rTriangle.m_Vertices[1], _Epsilon)) && (m_Vertices[2].IsEqual(_rTriangle.m_Vertices[2], _Epsilon));
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CTriangle3<T>::IsEqual(const CThis& _rTriangle, const CVector& _rEpsilon) const
    {
        return (m_Vertices[0].IsEqual(_rTriangle.m_Vertices[0], _rEpsilon)) && (m_Vertices[1].IsEqual(_rTriangle.m_Vertices[1], _rEpsilon)) && (m_Vertices[2].IsEqual(_rTriangle.m_Vertices[2], _rEpsilon));
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::CVector CTriangle3<T>::GetNormal() const
    {
        return ((m_Vertices[1] - m_Vertices[0]).CrossProduct(m_Vertices[2] - m_Vertices[0])).Normalize();
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline typename CTriangle3<T>::X CTriangle3<T>::GetArea() const
    {
        return X(0.5) * ((m_Vertices[1] - m_Vertices[0]).CrossProduct(m_Vertices[2] - m_Vertices[0])).Length();
    }

    // -------------------------------------------------------------------------------

    template <typename T>
    inline bool CTriangle3<T>::IntersectsRay(const CVector& _rOrigin, const CVector& _rDirection, CVector& _rHitPoint, X& _rLambda) const
    {
        X A;
        X F;
        X U;
        X V;
        X Lambda;

        // -------------------------------------------------------------------------------
        // Check if the ray is parallel to the plane of the triangle.
        // -------------------------------------------------------------------------------
        CVector Delta1 = m_Vertices[1] - m_Vertices[0];
        CVector Delta2 = m_Vertices[2] - m_Vertices[0];

        CVector P = _rDirection.CrossProduct(Delta2);

        A = Delta1.DotProduct(P);

        if (Base::IsEqual(A, X(0), Base::SConstants<X>::s_Epsilon))
        {
            return false;
        }

        // -------------------------------------------------------------------------------
        // Get the texture coordinate in u-direction.
        // -------------------------------------------------------------------------------
        F = X(1) / A;

        CVector Q = _rOrigin - m_Vertices[0];

        U = F * Q.DotProduct(P);

        if ((U < X(0)) || (U > X(1)))
        {
            return false;
        }

        // -------------------------------------------------------------------------------
        // Get the texture coordinate in v-direction.
        // -------------------------------------------------------------------------------
        CVector R = Q.CrossProduct(Delta1);

        V = F * _rDirection.DotProduct(R);

        if ((V < X(0)) || ((U + V) > X(1)))
        {
            return false;
        }

        // ------------------------------------------------------------------------
        // Check if the intersection is before the origin.
        // ------------------------------------------------------------------------
        Lambda = F * Delta2.DotProduct(R);

        if (Lambda < X(0))
        {
            return false;
        }

        // -----------------------------------------------------------------------------
        // Set return values
        // -----------------------------------------------------------------------------
        _rHitPoint = _rOrigin + (_rDirection * Lambda);
        _rLambda   = Lambda;

        return true;
    }
} // namespace MATH