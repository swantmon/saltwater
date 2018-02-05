
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_plane.h"

#include "base/base_lib_glm.h"

using namespace Base;

BASE_TEST(TestPlaneDistance)
{
    CPlane<float> MyPlane;
    
    MyPlane.Set(0.0f, 0.0f, 1.0f, 0.0f);
    
    BASE_CHECK(MyPlane.GetDistance(glm::vec3(1.0f)) == 1.0f);
    
    BASE_CHECK(MyPlane.GetDistance(glm::vec3(2.0f)) == 2.0f);
    
    BASE_CHECK(MyPlane.GetDistance(glm::vec3(-2.0f)) == -2.0f);
}

BASE_TEST(TestPlaneAgainstRay)
{
    CPlane<float> MyPlane;
    
    MyPlane.Set(0.0f, 0.0f, 1.0f, 0.0f);
    
    glm::vec3 IntersectionPoint;
    float     Lambda = 0.0f;
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyPlane.IntersectsRay(glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, -1.0f), IntersectionPoint, Lambda));
    
    BASE_CHECK(IntersectionPoint == glm::vec3(1.0f, 1.0f, 0.0f));
    
    BASE_CHECK(Lambda == 1.0f);
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyPlane.IntersectsRay(glm::vec3(-1.0f), glm::vec3(0.0f, 0.0f, 1.0f), IntersectionPoint, Lambda));
    
    BASE_CHECK(IntersectionPoint == glm::vec3(-1.0f, -1.0f, 0.0f));
    
    BASE_CHECK(Lambda == 1.0f);
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(!MyPlane.IntersectsRay(glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), IntersectionPoint, Lambda));
}