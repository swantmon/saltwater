
#include "base/base_test_defines.h"

#include "base/base_plane.h"

using namespace Base;

BASE_TEST(TestPlaneDistance)
{
    CPlane<float> MyPlane;
    
    MyPlane.Set(0.0f, 0.0f, 1.0f, 0.0f);
    
    BASE_CHECK(MyPlane.GetDistance(Float3(1.0f)) == 1.0f);
    
    BASE_CHECK(MyPlane.GetDistance(Float3(2.0f)) == 2.0f);
    
    BASE_CHECK(MyPlane.GetDistance(Float3(-2.0f)) == -2.0f);
}

BASE_TEST(TestPlaneAgainstRay)
{
    CPlane<float> MyPlane;
    
    MyPlane.Set(0.0f, 0.0f, 1.0f, 0.0f);
    
    Float3 IntersectionPoint;
    float  Lambda;
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyPlane.IntersectsRay(Float3(1.0f), Float3(0.0f, 0.0f, -1.0f), IntersectionPoint, Lambda));
    
    BASE_CHECK(IntersectionPoint == Float3(1.0f, 1.0f, 0.0f));
    
    BASE_CHECK(Lambda == 1.0f);
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyPlane.IntersectsRay(Float3(-1.0f), Float3(0.0f, 0.0f, 1.0f), IntersectionPoint, Lambda));
    
    BASE_CHECK(IntersectionPoint == Float3(-1.0f, -1.0f, 0.0f));
    
    BASE_CHECK(Lambda == 1.0f);
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(!MyPlane.IntersectsRay(Float3(1.0f), Float3(0.0f, 0.0f, 1.0f), IntersectionPoint, Lambda));
}