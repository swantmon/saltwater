
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_aabb3.h"

#include "base/base_sphere.h"

#include "base/base_lib_glm.h"

using namespace Base;

BASE_TEST(SphereCollisionWithAABB)
{
    CSphere<float> MySphere;
    CSphere<float> OutterSphere;
    CSphere<float> InnerSphere;
    CSphere<float> IntersectingSphere;
    CSphere<float> HullSphere;
    
    MySphere.Set(1.0f, 1.0f, 1.0f, 1.0f);
    
    OutterSphere.Set(-1.0f, -1.0f, -1.0f, 1.0f);
    
    InnerSphere.Set(1.0f, 1.0f, 1.0f, 0.5f);
    
    IntersectingSphere.Set(0.0f, 0.0f, 0.0f, 1.0f);
    
    HullSphere.Set(0.0f, 0.0f, 0.0f, 10.0f);
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(!MySphere.Contains(glm::vec3(0.0f)));
    
    BASE_CHECK(MySphere.Contains(glm::vec3(1.0f)));
    
    BASE_CHECK(MySphere.Contains(glm::vec3(1.0f, 1.0f, 0.0f)));
    
    BASE_CHECK(MySphere.Contains(glm::vec3(0.0f, 1.0f, 1.0f)));
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(!MySphere.Intersects(OutterSphere));
    
    BASE_CHECK(MySphere.Intersects(InnerSphere));
    
    BASE_CHECK(MySphere.Intersects(IntersectingSphere));
    
    BASE_CHECK(MySphere.Intersects(HullSphere));
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(!MySphere.Contains(OutterSphere));
    
    BASE_CHECK(MySphere.Contains(InnerSphere));
    
    BASE_CHECK(!MySphere.Contains(IntersectingSphere));
    
    BASE_CHECK(!MySphere.Contains(HullSphere) && HullSphere.Contains(MySphere));
}