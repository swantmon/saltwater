
#include "base/base_test_defines.h"

#include "base/base_math_constants.h"
#include "base/base_triangle3.h"

using namespace Base;

BASE_TEST(TestTriangle3Normal)
{
    CTriangle3<float> MyTriangle;
    
    MyTriangle.Set(Float3(0.0f), Float3(2.0f, 0.0f, 0.0f), Float3(1.0f, 1.0f, 0.0f));
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyTriangle.GetNormal() == Float3(0.0f, 0.0f, 1.0f));
    
    // -----------------------------------------------------------------------------
    
    MyTriangle.Set(Float3(0.0f), Float3(1.0f, 1.0f, 0.0f), Float3(2.0f, 0.0f, 0.0f));
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyTriangle.GetNormal() == Float3(0.0f, 0.0f, -1.0f));
}

BASE_TEST(TestTriangle3Equal)
{
    CTriangle3<float> MyTriangle;
    CTriangle3<float> MyTriangleSame;
    CTriangle3<float> MyTriangleIntersect;
    CTriangle3<float> MyTriangleOutter;
    
    MyTriangle.Set(Float3(0.0f), Float3(2.0f, 0.0f, 0.0f), Float3(1.0f, 1.0f, 0.0f));
    
    MyTriangleSame.Set(Float3(0.0f), Float3(1.0f, 1.0f, 0.0f), Float3(2.0f, 0.0f, 0.0f));
    
    MyTriangleIntersect.Set(Float3(0.5f, 0.0f, 0.0f), Float3(2.5f, 0.0f, 0.0f), Float3(1.5f, 1.5f, 0.0f));
    
    MyTriangleOutter.Set(Float3(0.0f, 0.0f, 1.0f), Float3(2.0f, 0.0f, 1.0f), Float3(1.0f, 1.0f, 1.0f));
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(!MyTriangle.IsEqual(MyTriangleSame, SConstants<float>::s_Epsilon));
    
    BASE_CHECK(!MyTriangle.IsEqual(MyTriangleIntersect, SConstants<float>::s_Epsilon));
    
    BASE_CHECK(!MyTriangle.IsEqual(MyTriangleOutter, SConstants<float>::s_Epsilon));
    
    BASE_CHECK( MyTriangle.IsEqual(MyTriangleOutter, 1.0f));
}


BASE_TEST(TestTriangle3Area)
{
    CTriangle3<float> MyTriangle;
    
    MyTriangle.Set(Float3(0.0f), Float3(1.0f, 1.0f, 0.0f), Float3(1.0f, 0.0f, 0.0f));
    
    BASE_CHECK(MyTriangle.GetArea() == 0.5f);
}

BASE_TEST(TestTriangle3AgainstRay)
{
    CTriangle3<float> MyTriangle;
    
    MyTriangle.Set(Float3(0.0f), Float3(1.0f, 1.0f, 0.0f), Float3(1.0f, 0.0f, 0.0f));
    
    Float3 IntersectionPoint;
    float  Lambda;
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyTriangle.IntersectsRay(Float3(1.0f), Float3(0.0f, 0.0f, -1.0f), IntersectionPoint, Lambda));
    
    BASE_CHECK(IntersectionPoint == Float3(1.0f, 1.0f, 0.0f));
    
    BASE_CHECK(Lambda == 1.0f);
    
    // -----------------------------------------------------------------------------
    
    BASE_CHECK(MyTriangle.IntersectsRay(Float3(1.0f, 1.0f, 2.0f), Float3(0.0f, 0.0f, -1.0f), IntersectionPoint, Lambda));
    
    BASE_CHECK(IntersectionPoint == Float3(1.0f, 1.0f, 0.0f));
    
    BASE_CHECK(Lambda == 2.0f);
}

