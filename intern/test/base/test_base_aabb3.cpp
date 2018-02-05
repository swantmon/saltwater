
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_aabb3.h"

#include "base/base_lib_glm.h"

BASE_TEST(Test_Base_AABB3_Intersect_AABB3)
{
    glm::vec3 MinPoint1(0,0,0);
    glm::vec3 MaxPoint1(10,10,10);

    Base::AABB3Float Box1(MinPoint1, MaxPoint1);

    // -----------------------------------------------------------------------------

    glm::vec3 MinPoint2(20,20,20);
    glm::vec3 MaxPoint2(30,30,30);

    Base::AABB3Float Box2(MinPoint2, MaxPoint2);

    // -----------------------------------------------------------------------------

    glm::vec3 MinPoint3(5,5,5);
    glm::vec3 MaxPoint3(8,8,8);

    Base::AABB3Float Box3(MinPoint3, MaxPoint3);

    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------

    BASE_CHECK( Box1.ContainsPoint(MinPoint3));
    BASE_CHECK(!Box1.ContainsPoint(MaxPoint2));

    BASE_CHECK( Box1.IntersectsAABB(Box3));
    BASE_CHECK(!Box1.IntersectsAABB(Box2));

    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------

    Base::AABB3Float IntersectionBox = Box1.Intersection(Box3);

    BASE_CHECK(IntersectionBox.GetMin() == MinPoint3);
    BASE_CHECK(IntersectionBox.GetMax() == MaxPoint3);

    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------

    Base::AABB3Float UnionBox = Box1.Union(Box2);

    BASE_CHECK(UnionBox.GetMin() == MinPoint1);
    BASE_CHECK(UnionBox.GetMax() == MaxPoint2);
}