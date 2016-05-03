
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_vector4.h"
#include "base/base_matrix4x4.h"


BASE_TEST(Test_Base_Vector4_Add)
{
    Base::Float4 Vector1(1, 2, 3, 4);

    Base::Float4 Vector2(4, 3, 2, 1);

    Base::Float4 VectorResult(5, 5, 5, 5);

    BASE_CHECK(VectorResult == Vector2 + Vector1);
    BASE_CHECK(VectorResult == Vector1 + Vector2);
}

BASE_TEST(Test_Base_Vector4_Mul)
{
    Base::Float4 Vector1(1, 2, 3, 4);

    Base::Float4 Vector2(4, 3, 2, 1);

    Base::Float4 VectorResult(4, 6, 6, 4);

    BASE_CHECK(VectorResult == Vector2 * Vector1);
    BASE_CHECK(VectorResult == Vector1 * Vector2);
}

BASE_TEST(Test_Base_Vector4_Mul_Matrix4)
{
    Base::Float4 Vector(0, 0, -1, 1);

    Base::Float4x4 ViewMatrix
        (
        1, 0, 0,  0,
        0, 1, 0,  0,
        0, 0, 1, -8,  
        0, 0, 0,  1
        );

    Base::Float4 VectorResult
        (
         0, 0, -9, 1
        );
    
    Base::Float4 VectorResultInverted
        (
         0, 0, -1, 9
        );

    BASE_CHECK(VectorResult         == ViewMatrix * Vector); // Right

    BASE_CHECK(VectorResult         != Vector * ViewMatrix); // Wrong!
    
    BASE_CHECK(VectorResultInverted == Vector * ViewMatrix); // Right
}

BASE_TEST(Test_Base_Vector4_Mul_Model_View_Vector)
{
    Base::Float4x4 ViewMatrix
        (
        1.0f, 0.0f, 0.0f, 0.0f,  
        0.0f, 1.0f, 0.0f, 0.0f,  
        0.0f, 0.0f, 1.0f, -8.0f,  
        0.0f, 0.0f, 0.0f, 1.0f
        );

    Base::Float4x4 ModelMatrix
        (
        1.0f, 0.0f, 0.0f, 0.0f,  
        0.0f, 1.0f, 0.0f, 3.06f,  
        0.0f, 0.0f, 1.0f, 0.0f,  
        0.0f, 0.0f, 0.0f, 1.0f
        );

    Base::Float4 Vector1(1, 2, 3, 1);

    Base::Float4 VectorResult(1.0f, 5.05999994f, -5.0f, 1.0f);
     
    BASE_CHECK(VectorResult ==  ViewMatrix * ModelMatrix * Vector1);
}

BASE_TEST(Test_Base_Vector4_Length)
{
    Base::Float4 Vector1(1, 2, 3, 4);

    BASE_CHECK(5.47722578f == Vector1.Length());
}

BASE_TEST(QuaterionToTangentSpaceAndBack)
{
    Base::Float3 Normal(1, 2, 3);
    
    Base::Float3 VectorNormal;
    
    Base::Float3 VectorBinormal;
    
    Base::Float3 VectorTangent;
    
    Base::Float3 CheckVector1;
    
    Base::Float3 CheckVector2;
    
    Base::Float3 CheckVector3;
    
    Base::Float4 Quaternion;
    
    auto CalculateWorldBase =[&] (Base::Float3 _NormalUnitDir, Base::Float3& _rBaseX, Base::Float3& _rBaseY, Base::Float3& _rBaseZ)
    {
        Base::Float3 NonCollinearAxis;
        
        // -----------------------------------------------------------------------------
        // Choose an axis of the world system which is not collinear to the normal.
        // -----------------------------------------------------------------------------
        NonCollinearAxis = (abs(_NormalUnitDir[1]) < 0.999f) ? Base::Float3(0.0f, 1.0f, 0.0f) : Base::Float3(0.0f, 0.0f, 1.0f);
        
        // -----------------------------------------------------------------------------
        // Calculate the world base.
        // -----------------------------------------------------------------------------
        _rBaseZ = _NormalUnitDir;
        _rBaseX = NonCollinearAxis.CrossProduct(_rBaseZ).Normalize();
        _rBaseY = _rBaseZ.CrossProduct(_rBaseX);
    };
    
    auto TangentSpaceToQuaternion = [&](Base::Float3 _Tangent, Base::Float3 _Binormal, Base::Float3 _Normal)->Base::Float4
    {
        Base::Float4 Quaternion;
        
        Quaternion[0] = _Normal[1]   - _Binormal[2];
        Quaternion[1] = _Tangent[2]  - _Normal[0];
        Quaternion[2] = _Binormal[0] - _Tangent[1];
        Quaternion[3] = 1.0f + _Tangent[0] + _Binormal[1] + _Normal[2];
        
        return Quaternion.Normalize();
    };
    
    auto QuaternionToTangentSpace = [&](Base::Float4 _Quaterion, Base::Float3& _rTangent, Base::Float3& _rBinormal, Base::Float3& _rNormal)->void
    {
        _rTangent = Base::Float3( 1.0f,  0.0f,  0.0f)
                  + Base::Float3(-2.0f,  2.0f,  2.0f) * _Quaterion[1] * Base::Float3(_Quaterion[1], _Quaterion[0], _Quaterion[3])
                  + Base::Float3(-2.0f, -2.0f,  2.0f) * _Quaterion[2] * Base::Float3(_Quaterion[2], _Quaterion[3], _Quaterion[0]);
        
        _rBinormal = Base::Float3( 0.0f,  1.0f,  0.0f)
                   + Base::Float3( 2.0f, -2.0f,  2.0f) * _Quaterion[2] * Base::Float3(_Quaterion[3], _Quaterion[2], _Quaterion[1])
                   + Base::Float3( 2.0f, -2.0f, -2.0f) * _Quaterion[0] * Base::Float3(_Quaterion[1], _Quaterion[0], _Quaterion[3]);
        
        _rNormal = Base::Float3( 0.0f,  0.0f,  1.0f)
                 + Base::Float3( 2.0f,  2.0f, -2.0f) * _Quaterion[0] * Base::Float3(_Quaterion[2], _Quaterion[3], _Quaterion[0])
                 + Base::Float3(-2.0f,  2.0f, -2.0f) * _Quaterion[1] * Base::Float3(_Quaterion[3], _Quaterion[2], _Quaterion[1]);
    };
    
    Normal = Normal.Normalize();
    
    CalculateWorldBase(Normal, VectorTangent, VectorBinormal, VectorNormal);
    
    Quaternion = TangentSpaceToQuaternion(VectorTangent, VectorBinormal, VectorNormal);
    
    QuaternionToTangentSpace(Quaternion, CheckVector1, CheckVector2, CheckVector3);
    
    BASE_CHECK(VectorTangent .IsEqual(CheckVector1, 0.0001f));
    BASE_CHECK(VectorBinormal.IsEqual(CheckVector2, 0.0001f));
    BASE_CHECK(VectorNormal  .IsEqual(CheckVector3, 0.0001f));
}