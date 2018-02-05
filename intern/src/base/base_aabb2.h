//
//  base_aabb2.h
//  base
//
//  Created by Tobias Schwandt on 29/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"

#include <assert.h>

#include "glm.hpp"

namespace MATH
{
    template<typename T>
    class CAABB2
    {
        //
        //  D +--------------+ C
        //    |              |
        //    |              |
        //    |              |
        //    |              |
        //    |              |
        //  A +--------------+ B
        //
        
    public:
        
        typedef CAABB2<T>    CThis;
        typedef T            X;
        typedef X&           XRef;
        typedef const X&     XConstRef;
        typedef X*           XPtr;
        typedef const X*     XConstPtr;
        
        typedef glm::vec2 CCorner;
        
    public:
        
        inline  CAABB2();
        inline  CAABB2(const CCorner& _rMinPoint, const CCorner& _rMaxPoint);
        inline  CAABB2(const CAABB2& _rCpy);
        inline ~CAABB2();
        
    public:
        
        inline CThis& operator = (const CThis& _rCpy);
        
    public:
        
        inline bool operator == (const CThis& _rAABB) const;
        inline bool operator != (const CThis& _rAABB) const;
        
        inline CCorner operator [] (unsigned int _Index) const;
        
    public:
        
        inline void Set(const CCorner& _rCenter, const CCorner& _rSize);
        
    public:
        
        inline CCorner& GetCenter();
        inline const CCorner& GetCenter() const;
        
        inline void SetMin(const CCorner& _rMin);
        inline CCorner& GetMin();
        inline const CCorner& GetMin() const;
        
        inline void SetMax(const CCorner& _rMin);
        inline CCorner& GetMax();
        inline const CCorner& GetMax() const;
        
    public:
        
        inline const CCorner BuildCorner(const glm::ivec2& _rIndices) const;
        
    public:
        
        inline bool Contains(const CCorner& _rCorner) const;
        inline bool Contains(const CThis& _rAABB) const;
        
        inline bool Intersects(const CThis& _rAABB) const;
        
    public:
        
        // Returns the minimal AABB which contains both AABBs.
        inline CThis Union(const CThis& _rAABB) const;
        
        // Returns the intersection of both AABBs.
        inline CThis Intersection(const CThis& _rAABB) const;
        
    public:
        
        inline CCorner Size() const;
        
    private:
        
#pragma warning(disable:4201)
        struct
        {
            union
            {
                struct
                {
                    CCorner m_MinPoint;
                    
                    CCorner m_MaxPoint;
                };
                
                T m_PointData[4];
            };
        };
#pragma warning(default:4201)
        
    private:
        
        inline bool IsValid() const;
        inline bool IsValid(const CCorner& _rMin, const CCorner& _rMax) const;
    };
} // namespace MATH

namespace MATH
{
    typedef CAABB2<int>          AABB2Int;
    typedef CAABB2<unsigned int> AABB2UInt;
    typedef CAABB2<float>        AABB2Float;
    typedef CAABB2<double>       AABB2Double;
} // namespace MATH

namespace MATH
{
    
    template<typename T>
    CAABB2<T>::CAABB2()
    : m_MinPoint(0)
    , m_MaxPoint(0)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CAABB2<T>::CAABB2(const CCorner& _rMinPoint, const CCorner& _rMaxPoint)
        : m_MinPoint(_rMinPoint)
        , m_MaxPoint(_rMaxPoint)
    {
        assert(IsValid(_rMinPoint, _rMaxPoint));
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CAABB2<T>::CAABB2(const CThis& _rCpy)
        : m_MinPoint(_rCpy.m_MinPoint)
        , m_MaxPoint(_rCpy.m_MaxPoint)
    {
        assert(_rCpy.IsValid());
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    CAABB2<T>::~CAABB2()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CThis& CAABB2<T>::operator=(const CThis& _rCpy)
    {
        assert(_rCpy.IsValid());
        
        this->m_MinPoint = _rCpy.m_MinPoint;
        this->m_MaxPoint = _rCpy.m_MaxPoint;
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::operator == (const CThis& _rAABB) const
    {
        return m_MinPoint == _rAABB.m_MinPoint && m_MaxPoint == _rAABB.m_MaxPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::operator != (const CThis& _rAABB) const
    {
        return m_MinPoint != _rAABB.m_MinPoint || m_MaxPoint != _rAABB.m_MaxPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CCorner CAABB2<T>::operator [] (unsigned int _Index) const
    {
        unsigned int Index = _Index * 2;
        
        return CCorner(m_PointData[Index + 0], m_PointData[Index + 1]);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CAABB2<T>::Set(const CCorner& _rCenter, const CCorner& _rSize)
    {
        CCorner HalfSize = _rSize / CCorner(2);
        
        m_MinPoint = _rCenter - HalfSize;
        m_MaxPoint = _rCenter + HalfSize;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CCorner& CAABB2<T>::GetCenter()
    {
        assert(IsValid());
        
        return (m_MinPoint + m_MaxPoint) / CCorner(2);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    const typename CAABB2<T>::CCorner& CAABB2<T>::GetCenter() const
    {
        assert(IsValid());
        
        return (m_MinPoint + m_MaxPoint) / CCorner(2);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CAABB2<T>::SetMin(const CCorner& _rMin)
    {
        m_MinPoint = _rMin;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CCorner& CAABB2<T>::GetMin()
    {
        return m_MinPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    const typename CAABB2<T>::CCorner& CAABB2<T>::GetMin() const
    {
        return m_MinPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void CAABB2<T>::SetMax(const CCorner& _rMax)
    {
        m_MaxPoint = _rMax;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CCorner& CAABB2<T>::GetMax()
    {
        return m_MaxPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    const typename CAABB2<T>::CCorner& CAABB2<T>::GetMax() const
    {
        return m_MaxPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    const typename CAABB2<T>::CCorner CAABB2<T>::BuildCorner(const glm::ivec2& _rIndices) const
    {
        assert(_rIndices[0] >=0 && _rIndices[0] < 4);
        assert(_rIndices[1] >=0 && _rIndices[1] < 4);
      
        return CCorner(m_PointData[_rIndices[0]], m_PointData[_rIndices[1]]);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::Contains(const CCorner& _rCorner) const
    {
        assert(IsValid());
        
        return (
                m_MinPoint[0] <= _rCorner[0] &&
                m_MinPoint[1] <= _rCorner[1] &&
                m_MaxPoint[0] >= _rCorner[0] &&
                m_MaxPoint[1] >= _rCorner[1]
                );
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::Contains(const CAABB2& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());
        
        return (
                m_MinPoint[0] <= _rAABB.m_MinPoint[0] &&
                m_MinPoint[1] <= _rAABB.m_MinPoint[1] &&
                m_MaxPoint[0] >= _rAABB.m_MaxPoint[0] &&
                m_MaxPoint[1] >= _rAABB.m_MaxPoint[1]
                );
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::Intersects(const CAABB2& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());
        
        return !(
                 m_MinPoint[0] >= _rAABB.m_MaxPoint[0] ||
                 m_MinPoint[1] >= _rAABB.m_MaxPoint[1] ||
                 m_MaxPoint[0] <= _rAABB.m_MinPoint[0] ||
                 m_MaxPoint[1] <= _rAABB.m_MinPoint[1]
                 );
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CThis CAABB2<T>::Union(const CThis& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());
        
        CThis UnionBox;
        
        UnionBox.m_MinPoint[0] = m_MinPoint[0] < _rAABB.m_MinPoint[0] ? m_MinPoint[0] : _rAABB.m_MinPoint[0];
        UnionBox.m_MinPoint[1] = m_MinPoint[1] < _rAABB.m_MinPoint[1] ? m_MinPoint[1] : _rAABB.m_MinPoint[1];
        
        UnionBox.m_MaxPoint[0] = m_MaxPoint[0] > _rAABB.m_MaxPoint[0] ? m_MaxPoint[0] : _rAABB.m_MaxPoint[0];
        UnionBox.m_MaxPoint[1] = m_MaxPoint[1] > _rAABB.m_MaxPoint[1] ? m_MaxPoint[1] : _rAABB.m_MaxPoint[1];
        
        assert(UnionBox.IsValid());
        
        return UnionBox;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CThis CAABB2<T>::Intersection(const CThis& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());
        
        CThis IntersectionBox;
        
        IntersectionBox.m_MinPoint[0] = m_MinPoint[0] > _rAABB.m_MinPoint[0] ? m_MinPoint[0] : _rAABB.m_MinPoint[0];
        IntersectionBox.m_MinPoint[1] = m_MinPoint[1] > _rAABB.m_MinPoint[1] ? m_MinPoint[1] : _rAABB.m_MinPoint[1];
        
        IntersectionBox.m_MaxPoint[0] = m_MaxPoint[0] < _rAABB.m_MaxPoint[0] ? m_MaxPoint[0] : _rAABB.m_MaxPoint[0];
        IntersectionBox.m_MaxPoint[1] = m_MaxPoint[1] < _rAABB.m_MaxPoint[1] ? m_MaxPoint[1] : _rAABB.m_MaxPoint[1];
        
        assert(IntersectionBox.IsValid());
        
        return IntersectionBox;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB2<T>::CCorner CAABB2<T>::Size() const
    {
        assert(IsValid());
        
        return m_MaxPoint - m_MinPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::IsValid() const
    {
        return m_MinPoint[0] <= m_MaxPoint[0] && m_MinPoint[1] <= m_MaxPoint[1];
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    bool CAABB2<T>::IsValid(const CCorner& _rMin, const CCorner& _rMax) const
    {
        return _rMin[0] <= _rMax[0] && _rMin[1] <= _rMax[1];
    }
    
} // namespace MATH