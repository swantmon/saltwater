
#pragma once

#include "base/base_aabb3.h"
#include "base/base_defines.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include <assert.h>

namespace MATH
{

	class CFrustum
    {

        //
        //    Definition: frustum is part of a solid, such as a cone or 
        //    pyramid, contained between the base and a plane parallel to 
        //    the base that intersects the solid 
        //    
        //    Defined by six clipping planes 
        //    (Left, Right, Bottom, Top, Near and Far)
        //     
        //            Top
        //      +-_-------------------_+ 
        //      |   \               /  |
        //      |    +------------+    |-> Near   
        //      |    |            |    |
        //      |    |            |----|-> Far
        //      |    |            |    |
        //  Left|    +------------+    |Right
        //      |   /              \   |  
        //      | /                  \ |   
        //      +----------------------+ 
        //           Bottom
        // 

    public:

        enum EClippingPlane
        {
            LEFT,
            RIGHT,
            BOTTOM,
            TOP,
            NEAR,
            FAR,
            NumberOfPlanes,
            UndefinedPlane = -1
        };

        enum EIntersects
        {
            OUTSIDE,
            INSIDE,
            INTERSECTING
        };

    public:

        inline glm::vec4& operator [] (const int _Index);
        inline const glm::vec4& operator [] (const int _Index) const;

    public:

        inline EIntersects Intersect(const Base::AABB3Float& _rAabb) const;

        inline void BuildIndices();

    private:

        glm::vec4 m_ClippingPlanes    [NumberOfPlanes];
        glm::ivec3   m_IndexMaxPlaneArray[NumberOfPlanes];
        glm::ivec3   m_IndexMinPlaneArray[NumberOfPlanes];

    private:

        inline EIntersects IntersectsClippingPlaneByAabb(Base::CFrustum::EClippingPlane _Plane, const Base::AABB3Float& _rAabb) const;


    };

} // namespace MATH

namespace MATH
{

	glm::vec4& CFrustum::operator[] (const int _Index)
    {
        assert(_Index >= 0 && _Index < NumberOfPlanes);

        return m_ClippingPlanes[_Index];
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CFrustum::operator[] (const int _Index) const
    {
        assert(_Index >= 0 && _Index < NumberOfPlanes);

        return m_ClippingPlanes[_Index];
    }

    // -----------------------------------------------------------------------------

    CFrustum::EIntersects CFrustum::Intersect(const Base::AABB3Float& _rAabb) const
    {
        bool IsIntersecting = false;

        EIntersects Intersects;

        Intersects = IntersectsClippingPlaneByAabb(Base::CFrustum::LEFT, _rAabb);

        if (Intersects == OUTSIDE)
        {
            return OUTSIDE;
        }
        else if (Intersects == INTERSECTING)
        {
            IsIntersecting = true;
        }

        Intersects = IntersectsClippingPlaneByAabb(Base::CFrustum::RIGHT, _rAabb);

        if (Intersects == OUTSIDE)
        {
            return OUTSIDE;
        }
        else if (Intersects == INTERSECTING)
        {
            IsIntersecting = true;
        }

        Intersects = IntersectsClippingPlaneByAabb(Base::CFrustum::TOP, _rAabb);

        if (Intersects == OUTSIDE)
        {
            return OUTSIDE;
        }
        else if (Intersects == INTERSECTING)
        {
            IsIntersecting = true;
        }

        Intersects = IntersectsClippingPlaneByAabb(Base::CFrustum::BOTTOM, _rAabb);

        if (Intersects == OUTSIDE)
        {
            return OUTSIDE;
        }
        else if (Intersects == INTERSECTING)
        {
            IsIntersecting = true;
        }

        Intersects = IntersectsClippingPlaneByAabb(Base::CFrustum::NEAR, _rAabb);

        if (Intersects == OUTSIDE)
        {
            return OUTSIDE;
        }
        else if (Intersects == INTERSECTING)
        {
            IsIntersecting = true;
        }

        Intersects = IntersectsClippingPlaneByAabb(Base::CFrustum::FAR, _rAabb);

        if (Intersects == OUTSIDE)
        {
            return OUTSIDE;
        }
        else if (Intersects == INTERSECTING)
        {
            IsIntersecting = true;
        }

        if (IsIntersecting)
        {
            return INTERSECTING;
        }

        return INSIDE;
    }

    // -----------------------------------------------------------------------------

    void CFrustum::BuildIndices()
    {
        for (unsigned int IndexOfClippingPlane = 0; IndexOfClippingPlane < NumberOfPlanes; IndexOfClippingPlane ++)
        {
            if (m_ClippingPlanes[IndexOfClippingPlane][0] < 0)
            {
                m_IndexMinPlaneArray[IndexOfClippingPlane][0] = 0;
                m_IndexMaxPlaneArray[IndexOfClippingPlane][0] = 3;
            }
            else
            {
                m_IndexMinPlaneArray[IndexOfClippingPlane][0] = 3;
                m_IndexMaxPlaneArray[IndexOfClippingPlane][0] = 0;
            }

            if (m_ClippingPlanes[IndexOfClippingPlane][1] < 0)
            {
                m_IndexMinPlaneArray[IndexOfClippingPlane][1] = 1;
                m_IndexMaxPlaneArray[IndexOfClippingPlane][1] = 4;
            }
            else
            {
                m_IndexMinPlaneArray[IndexOfClippingPlane][1] = 4;
                m_IndexMaxPlaneArray[IndexOfClippingPlane][1] = 1;
            }

            if (m_ClippingPlanes[IndexOfClippingPlane][2] < 0)
            {
                m_IndexMinPlaneArray[IndexOfClippingPlane][2] = 2;
                m_IndexMaxPlaneArray[IndexOfClippingPlane][2] = 5;
            }
            else
            {
                m_IndexMinPlaneArray[IndexOfClippingPlane][2] = 5;
                m_IndexMaxPlaneArray[IndexOfClippingPlane][2] = 2;
            }
        }
    }

    // -----------------------------------------------------------------------------

    CFrustum::EIntersects CFrustum::IntersectsClippingPlaneByAabb(Base::CFrustum::EClippingPlane _Plane, const Base::AABB3Float& _rAabb) const
    {
        glm::vec4 Plane    = m_ClippingPlanes[_Plane];
        glm::vec3 MinPoint = _rAabb.BuildCorner(m_IndexMinPlaneArray[_Plane]);
        glm::vec3 MaxPoint = _rAabb.BuildCorner(m_IndexMaxPlaneArray[_Plane]);

        float DistanceToMinimalPoint = 
            Plane[0] * MinPoint[0] +
            Plane[1] * MinPoint[1] +
            Plane[2] * MinPoint[2] +
            Plane[3];

        if (DistanceToMinimalPoint < 0)  return OUTSIDE;

        float DistanceToMaximalPoint = 
            Plane[0] * MaxPoint[0] +
            Plane[1] * MaxPoint[1] +
            Plane[2] * MaxPoint[2] +
            Plane[3];

        if (DistanceToMaximalPoint >= 0) return INSIDE;

        return INTERSECTING;
    }

} // namespace MATH