
#pragma once

#include "base/base_defines.h"
#include "base/base_include_glm.h"

#include <assert.h>

namespace MATH
{
    
    template<typename T>
    class CSphere
    {

        //     +
        //    /
        //   /     Radius
        //  +--------------+ Center
        //
        
    public:
        
        typedef CSphere<T> CThis;
        typedef T          X;
        typedef const X    XConst;
        typedef X&         XRef;
        typedef const X&   XConstRef;
        typedef X*         XPtr;
        typedef const X*   XConstPtr;
        
        typedef glm::tvec3<T> CVector;
        
    public:
        
        inline  CSphere();
        inline  CSphere(const CVector& _rCenter, X _Radius);
        inline  CSphere(X _AxisX, X _AxisY, X _AxisZ, X _Radius);
        inline  CSphere(const CSphere& _rCpy);
        inline ~CSphere();
        
    public:
        
        inline CThis& operator = (const CThis& _rCpy);
        
    public:
        
        inline bool operator == (const CThis& _rSphere) const;
        inline bool operator != (const CThis& _rSphere) const;
        
    public:
        
        inline void Set(const CVector& _rCenter, X _Radius);
        inline void Set(X _AxisX, X _AxisY, X _AxisZ, X _Radius);
        
    public:
        

        inline void SetCenter(const CVector& _rCenter);
        inline CVector& GetCenter();
        inline const CVector& GetCenter() const;
        
        inline void SetRadius(X _Radius);
        inline X GetRadius();
        
    public:
        
        inline bool Contains(const CVector& _rPoint) const;
        inline bool Contains(const CThis& _rSphere) const;
        
        inline bool Intersects(const CThis& _rSphere) const;
        
    private:
        
        CVector m_Center;
        X       m_Radius;
        
    };
    
} // namespace MATH

namespace MATH
{
    typedef CSphere<float>  SphereFloat;
    typedef CSphere<double> SphereDouble;
} // namespace MATH

namespace MATH
{
    template<typename T>
    CSphere<T>::CSphere()
        : m_Center()
        , m_Radius(X(0))
    {
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CSphere<T>::CSphere(const CVector& _rCenter, X _Radius)
        : m_Center(_rCenter)
        , m_Radius(_Radius)
    {
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CSphere<T>::CSphere(X _AxisX, X _AxisY, X _AxisZ, X _Radius)
        : m_Center(_AxisX, _AxisY, _AxisZ)
        , m_Radius(_Radius)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CSphere<T>::CSphere(const CSphere& _rCpy)
        : m_Center(_rCpy.m_Center)
        , m_Radius(_rCpy.m_Radius)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CSphere<T>::~CSphere()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CSphere<T>::CThis& CSphere<T>::operator = (const CThis& _rCpy)
    {
        m_Center = _rCpy.m_Center;
        m_Radius = _rCpy.m_Radius;

        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CSphere<T>::operator == (const CThis& _rSphere) const
    {
        return (m_Center == _rSphere.m_Center) && (m_Radius == _rSphere.m_Radius);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CSphere<T>::operator != (const CThis& _rSphere) const
    {
        return (m_Center != _rSphere.m_Center) || (m_Radius != _rSphere.m_Radius);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CSphere<T>::Set(const CVector& _rCenter, X _Radius)
    {
        m_Center = _rCenter;
        m_Radius = _Radius;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CSphere<T>::Set(X _AxisX, X _AxisY, X _AxisZ, X _Radius)
    {
        m_Center[0] = _AxisX;
        m_Center[1] = _AxisY;
        m_Center[2] = _AxisZ;
        m_Radius    = _Radius;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CSphere<T>::SetCenter(const CVector& _rCenter)
    {
        m_Center = _rCenter;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CSphere<T>::CVector& CSphere<T>::GetCenter()
    {
        return m_Center;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    const typename  CSphere<T>::CVector& CSphere<T>::GetCenter() const
    {
        return m_Center;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CSphere<T>::SetRadius(X _Radius)
    {
        m_Radius = _Radius;
    }
    
    // -----------------------------------------------------------------------------

    template<typename T>
    typename CSphere<T>::X CSphere<T>::GetRadius()
    {
        return m_Radius;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CSphere<T>::Contains(const CVector& _rPoint) const
    {
        auto Distance = (m_Center - _rPoint).length();
        
        return Distance <= m_Radius;
    }
    
    // -----------------------------------------------------------------------------

    template<typename T>
    bool CSphere<T>::Contains(const CThis& _rSphere) const
    {
        auto Distance        = (m_Center - _rSphere.m_Center).length();
        auto MinimalDistance = m_Radius - _rSphere.m_Radius;
        
        return Distance <= MinimalDistance;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CSphere<T>::Intersects(const CThis& _rSphere) const
    {
        auto Distance        = (m_Center - _rSphere.m_Center).length();
        auto MinimalDistance = m_Radius + _rSphere.m_Radius;
        
        return Distance <= MinimalDistance;
    }
    
} // namespace MATH
