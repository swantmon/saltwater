
#include "base/base_test_defines.h"

#include "base/base_matrix3x3.h"
#include "base/base_matrix4x4.h"
#include "base/base_quaternion.h"


BASE_TEST(Test_Base_Quaternion_Rotation_Matrix)
{
    Base::CQuaternion<float> TestQuat;
    
    // -----------------------------------------------------------------------------
    
    TestQuat.Set(Base::Float3(1.0f, 0.0f, 0.0f), Base::DegreesToRadians(90.0f));
    
    Base::Float3x3 QuatXRotation90 = TestQuat.GetRHRotationMatrix();
    
    Base::Float3x3 XRotation90;
    
    XRotation90.SetRotationX(Base::DegreesToRadians(90.0f));
                    
    BASE_CHECK(QuatXRotation90.IsEqual(XRotation90, 0.000001f));
    
    // -----------------------------------------------------------------------------
    
    TestQuat.Set(Base::Float3(0.0f, 1.0f, 0.0f), Base::DegreesToRadians(90.0f));
    
    Base::Float3x3 QuatYRotation90 = TestQuat.GetRHRotationMatrix();
    
    Base::Float3x3 YRotation90;
    
    YRotation90.SetRotationY(Base::DegreesToRadians(90.0f));
    
    BASE_CHECK(QuatYRotation90.IsEqual(YRotation90, 0.000001f));
    
    // -----------------------------------------------------------------------------
    
    TestQuat.Set(Base::Float3(0.0f, 0.0f, 1.0f), Base::DegreesToRadians(90.0f));
    
    Base::Float3x3 QuatZRotation90 = TestQuat.GetRHRotationMatrix();
    
    Base::Float3x3 ZRotation90;
    
    ZRotation90.SetRotationZ(Base::DegreesToRadians(90.0f));
    
    BASE_CHECK(QuatZRotation90.IsEqual(ZRotation90, 0.000001f));
}

BASE_TEST(Test_Base_Quaternion_Lerp)
{
    Base::CQuaternion<float> TestQuatXRot;
    Base::CQuaternion<float> TestQuatYRot;
    Base::CQuaternion<float> TestQuatLerped;
    
    TestQuatXRot.Set(Base::Float3(1.0f, 0.0f, 0.0f), Base::DegreesToRadians(90.0f));
    TestQuatYRot.Set(Base::Float3(0.0f, 1.0f, 0.0f), Base::DegreesToRadians(90.0f));
    
    TestQuatLerped = TestQuatXRot.GetSLerp(TestQuatYRot, 0.0f);
    
    BASE_CHECK(TestQuatXRot.IsEqual(TestQuatLerped, 0.0001f));
    
    TestQuatLerped = TestQuatXRot.GetSLerp(TestQuatYRot, 1.0f);
    
    BASE_CHECK(TestQuatYRot.IsEqual(TestQuatLerped, 0.0001f));
}