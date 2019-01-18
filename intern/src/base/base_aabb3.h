
#pragma once

#include "base/base_defines.h"
#include "base/base_include_glm.h"

#include <assert.h>

namespace MATH
{

    template<typename T>
	class CAABB3
    {

        //
        //      H  +--------------+ G
        //        /|             /|
        //       / |            / |
        //      /  |           /  |
        //     /   |          /   |
        //  E +--------------+ F  |
        //    |  D +---------|--- + C
        //    |   /          |   /
        //    |  /           |  / 
        //    | /            | /  
        //    |/             |/   
        //  A +--------------+ B
        //

    public:

        typedef CAABB3<T>    CThis;
        typedef T            X;
        typedef X&           XRef;
        typedef const X&     XConstRef;
        typedef X*           XPtr;
        typedef const X*     XConstPtr;

        typedef glm::tvec3<T> CVector;

    public:

        inline  CAABB3();
        inline  CAABB3(const CVector& _rMinPoint, const CVector& _rMaxPoint);
        inline  CAABB3(const CAABB3& _rCpy);
        inline ~CAABB3();

    public:

        inline CThis& operator = (const CThis& _rCpy);

    public:

        inline bool operator == (const CThis& _rAABB) const;
        inline bool operator != (const CThis& _rAABB) const;
        
        inline CVector operator [] (unsigned int _Index) const;

    public:

        inline void Set(const CVector& _rCenter, const CVector& _rSize);

    public:

        inline CVector GetCenter();
        inline const CVector& GetCenter() const;

        inline void SetMin(const CVector& _rMin);
        inline CVector& GetMin();
        inline const CVector& GetMin() const;

        inline void SetMax(const CVector& _rMin);
        inline CVector& GetMax();
        inline const CVector& GetMax() const;
        
        inline void Extend(const CVector& _rVector);
        inline void StickyExtend(const CVector& _rVector, float _Factor);

    public:

        inline const CVector BuildCorner(const glm::ivec3& _rIndices) const;

    public:

        inline bool ContainsPoint(const CVector& _rPoint) const;
        inline bool ContainsAABB(const CThis& _rAABB) const;

        inline bool IntersectsAABB(const CThis& _rAABB) const;
        inline bool IntersectsRay(const CVector& _rOrigin, const CVector& _rDirection, CVector& _rHitPoint, X& _rLambda) const;

    public:

        // Returns the minimal AABB which contains both AABBs.
        inline CThis Union(const CThis& _rAABB) const;
        
        // Returns the intersection of both AABBs.
        inline CThis Intersection(const CThis& _rAABB) const;

    public:

        inline CVector GetSize() const;

    private:

        enum
        {
            MinX = 0,
            MinY = 1,
            MinZ = 2,
            MaxX = 3,
            MaxY = 4,
            MaxZ = 5,
        };

    private:

#pragma warning(push)
#pragma warning(disable:4201)
        struct  
        {
            union 
            {
                struct 
                {
                    CVector m_MinPoint;

                    CVector m_MaxPoint;
                };

                X m_V[6];
            };
        };
#pragma warning(pop)

    private:

        inline bool IsValid() const;
        inline bool IsValid(const CVector& _rMin, const CVector& _rMax) const;

    };

} // namespace MATH

namespace MATH
{

    typedef CAABB3<int>    AABB3int;
	typedef CAABB3<float>  AABB3Float;
    typedef CAABB3<double> AABB3Double;

} // namespace MATH

namespace MATH
{

    template<typename T>
    CAABB3<T>::CAABB3()
        : m_MinPoint(0)
        , m_MaxPoint(0)
    {

    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CAABB3<T>::CAABB3(const CVector& _rMinPoint, const CVector& _rMaxPoint)
        : m_MinPoint(_rMinPoint)
        , m_MaxPoint(_rMaxPoint)
    {
        assert(IsValid(_rMinPoint, _rMaxPoint));
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CAABB3<T>::CAABB3(const CThis& _rCpy)
        : m_MinPoint(_rCpy.m_MinPoint)
        , m_MaxPoint(_rCpy.m_MaxPoint)
    {
        assert(_rCpy.IsValid());
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CAABB3<T>::~CAABB3()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CThis& CAABB3<T>::operator=(const CThis& _rCpy)
    {
        assert(_rCpy.IsValid());

        this->m_MinPoint = _rCpy.m_MinPoint;
        this->m_MaxPoint = _rCpy.m_MaxPoint;

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::operator == (const CThis& _rAABB) const
    {
        return m_MinPoint == _rAABB.m_MinPoint && m_MaxPoint == _rAABB.m_MaxPoint;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::operator != (const CThis& _rAABB) const
    {
        return m_MinPoint != _rAABB.m_MinPoint || m_MaxPoint != _rAABB.m_MaxPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    typename CAABB3<T>::CVector CAABB3<T>::operator [] (unsigned int _Index) const
    {
        unsigned int Index = _Index * 3;
        
        return CVector(m_V[Index + 0], m_V[Index + 1], m_V[Index + 2]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CAABB3<T>::Set(const CVector& _rCenter, const CVector& _rSize)
    {
        CVector HalfSize = _rSize / CVector(2);

        m_MinPoint = _rCenter - HalfSize;
        m_MaxPoint = _rCenter + HalfSize;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CVector CAABB3<T>::GetCenter()
    {
        assert(IsValid());

        return (m_MinPoint + m_MaxPoint) / CVector(2);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const typename CAABB3<T>::CVector& CAABB3<T>::GetCenter() const
    {
        assert(IsValid());

        return (m_MinPoint + m_MaxPoint) / CVector(2);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CAABB3<T>::SetMin(const CVector& _rMin)
    {
        m_MinPoint = _rMin;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CVector& CAABB3<T>::GetMin()
    {
        return m_MinPoint;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const typename CAABB3<T>::CVector& CAABB3<T>::GetMin() const
    {
        return m_MinPoint;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CAABB3<T>::SetMax(const CVector& _rMax)
    {
        m_MaxPoint = _rMax;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CVector& CAABB3<T>::GetMax()
    {
        return m_MaxPoint;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const typename CAABB3<T>::CVector& CAABB3<T>::GetMax() const
    {
        return m_MaxPoint;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void CAABB3<T>::Extend(const CVector& _rVector)
    {
        if (ContainsPoint(_rVector)) return;
        
        m_MinPoint[0] = m_MinPoint[0] < _rVector[0] ? m_MinPoint[0] : _rVector[0];
        m_MinPoint[1] = m_MinPoint[1] < _rVector[1] ? m_MinPoint[1] : _rVector[1];
        m_MinPoint[2] = m_MinPoint[2] < _rVector[2] ? m_MinPoint[2] : _rVector[2];
        
        m_MaxPoint[0] = m_MaxPoint[0] > _rVector[0] ? m_MaxPoint[0] : _rVector[0];
        m_MaxPoint[1] = m_MaxPoint[1] > _rVector[1] ? m_MaxPoint[1] : _rVector[1];
        m_MaxPoint[2] = m_MaxPoint[2] > _rVector[2] ? m_MaxPoint[2] : _rVector[2];
        
        assert(IsValid());
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    inline void CAABB3<T>::StickyExtend(const CVector& _rVector, float _Factor)
    {
        if (ContainsPoint(_rVector)) return;

        glm::vec3 PointOnBox;
        float Distance;
        
        if(!IntersectsRay(_rVector, glm::normalize(GetCenter() - _rVector), PointOnBox, Distance)) return;

        glm::vec3 NDirection = glm::normalize(_rVector - PointOnBox);

        Distance = glm::clamp(_Factor / Distance, 0.0f, 1.0f) * Distance;

        glm::vec3 SloppyVector = PointOnBox + Distance * NDirection;

        Extend(SloppyVector);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const typename CAABB3<T>::CVector CAABB3<T>::BuildCorner(const glm::ivec3& _rIndices) const
    {
        assert(_rIndices[0] >=0 && _rIndices[0] < 6);
        assert(_rIndices[1] >=0 && _rIndices[1] < 6);
        assert(_rIndices[2] >=0 && _rIndices[2] < 6);

        return CVector(m_V[_rIndices[0]], m_V[_rIndices[1]], m_V[_rIndices[2]]);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::ContainsPoint(const CVector& _rPoint) const
    {
        assert(IsValid());

        return (
            m_MinPoint[0] <= _rPoint[0] && 
            m_MinPoint[1] <= _rPoint[1] && 
            m_MinPoint[2] <= _rPoint[2] &&
            m_MaxPoint[0] >= _rPoint[0] && 
            m_MaxPoint[1] >= _rPoint[1] && 
            m_MaxPoint[2] >= _rPoint[2] 
        );
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::ContainsAABB(const CAABB3& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());

        return (
            m_MinPoint[0] <= _rAABB.m_MinPoint[0] && 
            m_MinPoint[1] <= _rAABB.m_MinPoint[1] && 
            m_MinPoint[2] <= _rAABB.m_MinPoint[2] &&
            m_MaxPoint[0] >= _rAABB.m_MaxPoint[0] && 
            m_MaxPoint[1] >= _rAABB.m_MaxPoint[1] && 
            m_MaxPoint[2] >= _rAABB.m_MaxPoint[2] 
        );
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::IntersectsAABB(const CAABB3& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());

        return !(
            m_MinPoint[0] >= _rAABB.m_MaxPoint[0] || 
            m_MinPoint[1] >= _rAABB.m_MaxPoint[1] || 
            m_MinPoint[2] >= _rAABB.m_MaxPoint[2] ||
            m_MaxPoint[0] <= _rAABB.m_MinPoint[0] || 
            m_MaxPoint[1] <= _rAABB.m_MinPoint[1] || 
            m_MaxPoint[2] <= _rAABB.m_MinPoint[2] 
        );
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::IntersectsRay(const CVector& _rOrigin, const CVector& _rDirection, CVector& _rHitPoint, X& _rLambda) const
    {
        bool         IsInside;
        unsigned int Axis;
        X            Lambda[3];

        IsInside = true;

        Lambda[0] = X(-1);
        Lambda[1] = X(-1);
        Lambda[2] = X(-1);

        CVector HitPoint;

        if (_rOrigin[0] < m_V[MinX])
        {
            HitPoint[0] = m_V[MinX]; IsInside = false;

            if (!glm::epsilonEqual(_rDirection[0], X(0), glm::epsilon<float>()))
            {
                Lambda[0] = (m_V[MinX] - _rOrigin[0]) / _rDirection[0];
            }
        }
        else if (_rOrigin[0] > m_V[MaxX])
        {
            HitPoint[0] = m_V[MaxX]; IsInside = false;

            if (!glm::epsilonEqual(_rDirection[0], X(0), glm::epsilon<float>()))
            {
                Lambda[0] = (m_V[MaxX] - _rOrigin[0]) / _rDirection[0];
            }
        }

        if (_rOrigin[1] < m_V[MinY])
        {
            HitPoint[1] = m_V[MinY]; IsInside = false;

            if (!glm::epsilonEqual(_rDirection[1], X(0), glm::epsilon<float>()))
            {
                Lambda[1] = (m_V[MinY] - _rOrigin[1]) / _rDirection[1];
            }
        }
        else if (_rOrigin[1] > m_V[MaxY])
        {
            HitPoint[1] = m_V[MaxY]; IsInside = false;

            if (!glm::epsilonEqual(_rDirection[1], X(0), glm::epsilon<float>()))
            {
                Lambda[1] = (m_V[MaxY] - _rOrigin[1]) / _rDirection[1];
            }
        }

        if (_rOrigin[2] < m_V[MinZ])
        {
            HitPoint[2] = m_V[MinZ]; IsInside = false;

            if (!glm::epsilonEqual(_rDirection[2], X(0), glm::epsilon<float>()))
            {
                Lambda[2] = (m_V[MinZ] - _rOrigin[2]) / _rDirection[2];
            }
        }
        else if (_rOrigin[2] > m_V[MaxZ])
        {
            HitPoint[2] = m_V[MaxZ]; IsInside = false;

            if (!glm::epsilonEqual(_rDirection[2], X(0), glm::epsilon<float>()))
            {
                Lambda[2] = (m_V[MaxZ] - _rOrigin[2]) / _rDirection[2];
            }
        }

        // -------------------------------------------------------------------------------
        // Check if the origin is inside.
        // -------------------------------------------------------------------------------
        if (IsInside)
        {
            HitPoint = _rOrigin; _rLambda = X(0);

            return true;
        }

        // -------------------------------------------------------------------------------
        // Get the axis with the largest lambda.
        // -------------------------------------------------------------------------------
        Axis = 0;

        if (Lambda[1] > Lambda[Axis]) Axis = 1;
        if (Lambda[2] > Lambda[Axis]) Axis = 2;

        // -------------------------------------------------------------------------------
        // Check if at least the largest lambda is valid.
        // -------------------------------------------------------------------------------
        if (Lambda[Axis] < X(0))
        {
            return false;
        }

        switch (Axis)
        {
            case 0:
            {
                HitPoint[1] = _rOrigin[1] + (_rDirection[1] * Lambda[0]);

                if ((HitPoint[1] < m_V[MinY]) || (HitPoint[1] > m_V[MaxY]))
                {
                    return false;
                }

                HitPoint[2] = _rOrigin[2] + (_rDirection[2] * Lambda[0]);

                if ((HitPoint[2] < m_V[MinZ]) || (HitPoint[2] > m_V[MaxZ]))
                {
                    return false;
                }
            }
            break;

            case 1:
            {
                HitPoint[0] = _rOrigin[0] + (_rDirection[0] * Lambda[1]);

                if ((HitPoint[0] < m_V[MinX]) || (HitPoint[0] > m_V[MaxX]))
                {
                    return false;
                }

                HitPoint[2] = _rOrigin[2] + (_rDirection[2] * Lambda[1]);

                if ((HitPoint[2] < m_V[MinZ]) || (HitPoint[2] > m_V[MaxZ]))
                {
                    return false;
                }
            }
            break;

            case 2:
            {
                HitPoint[0] = _rOrigin[0] + (_rDirection[0] * Lambda[2]);

                if ((HitPoint[0] < m_V[MinX]) || (HitPoint[0] > m_V[MaxX]))
                {
                    return false;
                }

                HitPoint[1] = _rOrigin[1] + (_rDirection[1] * Lambda[2]);

                if ((HitPoint[1] < m_V[MinY]) || (HitPoint[1] > m_V[MaxY]))
                {
                    return false;
                }
            }
            break;
        }

        // -----------------------------------------------------------------------------
        // Set return data
        // -----------------------------------------------------------------------------
        _rHitPoint = HitPoint; 
        _rLambda   = Lambda[Axis];

        return true;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CThis CAABB3<T>::Union(const CThis& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());

        CThis UnionBox;

        UnionBox.m_MinPoint[0] = m_MinPoint[0] < _rAABB.m_MinPoint[0] ? m_MinPoint[0] : _rAABB.m_MinPoint[0];
        UnionBox.m_MinPoint[1] = m_MinPoint[1] < _rAABB.m_MinPoint[1] ? m_MinPoint[1] : _rAABB.m_MinPoint[1];
        UnionBox.m_MinPoint[2] = m_MinPoint[2] < _rAABB.m_MinPoint[2] ? m_MinPoint[2] : _rAABB.m_MinPoint[2];

        UnionBox.m_MaxPoint[0] = m_MaxPoint[0] > _rAABB.m_MaxPoint[0] ? m_MaxPoint[0] : _rAABB.m_MaxPoint[0];
        UnionBox.m_MaxPoint[1] = m_MaxPoint[1] > _rAABB.m_MaxPoint[1] ? m_MaxPoint[1] : _rAABB.m_MaxPoint[1];
        UnionBox.m_MaxPoint[2] = m_MaxPoint[2] > _rAABB.m_MaxPoint[2] ? m_MaxPoint[2] : _rAABB.m_MaxPoint[2];

        assert(UnionBox.IsValid());

        return UnionBox;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CThis CAABB3<T>::Intersection(const CThis& _rAABB) const
    {
        assert(IsValid() && _rAABB.IsValid());

        CThis IntersectionBox;

        IntersectionBox.m_MinPoint[0] = m_MinPoint[0] > _rAABB.m_MinPoint[0] ? m_MinPoint[0] : _rAABB.m_MinPoint[0];
        IntersectionBox.m_MinPoint[1] = m_MinPoint[1] > _rAABB.m_MinPoint[1] ? m_MinPoint[1] : _rAABB.m_MinPoint[1];
        IntersectionBox.m_MinPoint[2] = m_MinPoint[2] > _rAABB.m_MinPoint[2] ? m_MinPoint[2] : _rAABB.m_MinPoint[2];

        IntersectionBox.m_MaxPoint[0] = m_MaxPoint[0] < _rAABB.m_MaxPoint[0] ? m_MaxPoint[0] : _rAABB.m_MaxPoint[0];
        IntersectionBox.m_MaxPoint[1] = m_MaxPoint[1] < _rAABB.m_MaxPoint[1] ? m_MaxPoint[1] : _rAABB.m_MaxPoint[1];
        IntersectionBox.m_MaxPoint[2] = m_MaxPoint[2] < _rAABB.m_MaxPoint[2] ? m_MaxPoint[2] : _rAABB.m_MaxPoint[2];

        assert(IntersectionBox.IsValid());

        return IntersectionBox;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CAABB3<T>::CVector CAABB3<T>::GetSize() const
    {
        assert(IsValid());

        return m_MaxPoint - m_MinPoint;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::IsValid() const
    {
        return m_MinPoint[0] <= m_MaxPoint[0] && m_MinPoint[1] <= m_MaxPoint[1] && m_MinPoint[2] <= m_MaxPoint[2];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CAABB3<T>::IsValid(const CVector& _rMin, const CVector& _rMax) const
    {
        return _rMin[0] <= _rMax[0] && _rMin[1] <= _rMax[1] && _rMin[2] <= _rMax[2];
    }

} // namespace MATH