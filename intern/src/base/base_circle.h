
#pragma once

#include "base/base_aabb2.h"
#include "base/base_defines.h"
#include "base/base_vector2.h"

#include <assert.h>

namespace MATH
{
    template<typename T>
    class CCircle
    {
        //    
        //        Radius
        //  +--------------+ Center
        //
        
    public:

        typedef CCircle<T>   CThis;
        typedef T            X;
        typedef const X      XConst;
        typedef X&           XRef;
        typedef const X&     XConstRef;
        typedef X*           XPtr;
        typedef const X*     XConstPtr;

        typedef CVector2<T>  CVector;

    public:

        inline  CCircle();
        inline  CCircle(const CVector& _rCenter, X _Radius);
        inline  CCircle(X _X, X _Y, X _Radius);
        inline  CCircle(const CCircle& _rCpy);
        inline ~CCircle();

    public:

        inline CThis& operator = (const CThis& _rCpy);

    public:

        inline bool operator == (const CThis& _rCircle) const;
        inline bool operator != (const CThis& _rCircle) const;

    public:

        inline void Set(const CThis& _rCircle);
        inline void Set(const CVector& _rCenter, X _Radius);
        inline void Set(X _X, X _Y, X _Radius);

    public:


        inline void SetCenter(const CVector& _rCenter);
        inline CVector& GetCenter();
        inline const CVector& GetCenter() const;

        inline void SetRadius(X _Radius);
        inline X GetRadius();

    public:

        inline bool ContainsPoint(const CVector& _rPoint) const;
        inline bool ContainsCircle(const CThis& _rCircle) const;

        inline bool IntersectsCircle(const CThis& _rCircle) const;
        inline bool IntersectsAABB(const CAABB2<X>& _rAABB) const;

    private:

        CVector m_Center;
        X       m_Radius;

    };

} // namespace MATH

namespace MATH
{
    template<typename T>
    CCircle<T>::CCircle()
        : m_Center()
        , m_Radius(X(0))
    {
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CCircle<T>::CCircle(const CVector& _rCenter, X _Radius)
        : m_Center(_rCenter)
        , m_Radius(_Radius)
    {
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CCircle<T>::CCircle<T>(X _X, X _Y, X _Radius)
        : m_Center(_X, _Y)
        , m_Radius(_Radius)
    {
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CCircle<T>::CCircle(const CCircle& _rCpy)
        : m_Center(_rCpy.m_Center)
        , m_Radius(_rCpy.m_Radius)
    {

    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CCircle<T>::~CCircle()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CCircle<T>::CThis& CCircle<T>::operator = (const CThis& _rCpy)
    {
        m_Center = _rCpy.m_Center;
        m_Radius = _rCpy.m_Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CCircle<T>::operator == (const CThis& _rCircle) const
    {
        return (m_Center == _rCircle.m_Center) && (m_Radius == _rCircle.m_Radius);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CCircle<T>::operator != (const CThis& _rCircle) const
    {
        return (m_Center != _rCircle.m_Center) || (m_Radius != _rCircle.m_Radius);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CCircle<T>::Set(const CThis& _rCircle)
    {
        m_Center = _rCircle.m_Center;
        m_Radius = _rCircle.m_Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CCircle<T>::Set(const CVector& _rCenter, X _Radius)
    {
        m_Center = _rCenter;
        m_Radius = _Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CCircle<T>::Set(X _X, X _Y, X _Radius)
    {
        m_Center[0] = _X;
        m_Center[1] = _Y;
        m_Radius = _Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CCircle<T>::SetCenter(const CVector& _rCenter)
    {
        m_Center = _rCenter;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CCircle<T>::CVector& CCircle<T>::GetCenter()
    {
        return m_Center;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const typename  CCircle<T>::CVector& CCircle<T>::GetCenter() const
    {
        return m_Center;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CCircle<T>::SetRadius(X _Radius)
    {
        m_Radius = _Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CCircle<T>::X CCircle<T>::GetRadius()
    {
        return m_Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CCircle<T>::ContainsPoint(const CVector& _rPoint) const
    {
        X Distance = (m_Center - _rPoint).GetLength();

        return Distance <= m_Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CCircle<T>::ContainsCircle(const CThis& _rCircle) const
    {
        X Distance = (m_Center - _rCircle.m_Center).GetLength();
        X MinimalDistance = m_Radius - _rCircle.m_Radius;

        return Distance <= MinimalDistance;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CCircle<T>::IntersectsCircle(const CThis& _rCircle) const
    {
        X Distance = (m_Center - _rCircle.m_Center).GetLength();
        X MinimalDistance = m_Radius + _rCircle.m_Radius;

        return Distance <= MinimalDistance;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CCircle<T>::IntersectsAABB(const CAABB2<X>& _rAABB) const
    {
        X Distance;
        X SumOfDistances;

        SumOfDistances = X(0);

        if ((Distance = m_Center[0] - _rAABB.GetMin()[0]) < X(0))
        {
            if (Distance < -m_Radius)
            {
                return false;
            }

            SumOfDistances += Distance * Distance;
        }
        else if ((Distance = m_Center[0] - _rAABB.GetMax()[0]) > X(0))
        {
            if (Distance > m_Radius)
            {
                return false;
            }

            SumOfDistances += Distance * Distance;
        }

        if ((Distance = m_Center[1] - _rAABB.GetMin()[1]) < X(0))
        {
            if (Distance < -m_Radius)
            {
                return false;
            }

            SumOfDistances += Distance * Distance;
        }
        else if ((Distance = m_Center[1] - _rAABB.GetMax()[1]) > X(0))
        {
            if (Distance > m_Radius)
            {
                return false;
            }

            SumOfDistances += Distance * Distance;
        }

        return SumOfDistances <= (m_Radius * m_Radius);
    }

} // namespace MATH