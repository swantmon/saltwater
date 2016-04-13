
#include "base/base_test_defines.h"

#include "base/base_matrix4x4.h"


BASE_TEST(Test_Base_Matrix4_Add)
{
    Base::Float4x4 Matrix1
        (
        1,1,1,1,  2,2,2,2,  3,3,3,3,  4,4,4,4
        );

    Base::Float4x4 Matrix2
        (
        4,4,4,4,  3,3,3,3,  2,2,2,2,  1,1,1,1
        );

    Base::Float4x4 MatrixResult
        (
        5,5,5,5,  5,5,5,5,  5,5,5,5,  5,5,5,5
        );

    BASE_CHECK(MatrixResult == Matrix2 + Matrix1);
    BASE_CHECK(MatrixResult == Matrix1 + Matrix2);
}

BASE_TEST(Test_Base_Matrix4_Mul)
{
    Base::Float4x4 Matrix1
        (
        1,2,3,4,  
        1,2,3,4, 
        1,2,3,4,  
        1,2,3,4
        );

    Base::Float4x4 Matrix2
        (
        4,3,2,1,
        4,3,2,1,
        4,3,2,1,
        4,3,2,1
        );

    Base::Float4x4 MatrixResult
        (
        40,30,20,10,  
        40,30,20,10, 
        40,30,20,10,   
        40,30,20,10
        );

    BASE_CHECK(MatrixResult == Matrix1 * Matrix2);
    BASE_CHECK(MatrixResult != Matrix2 * Matrix1);
}

BASE_TEST(Test_Base_Matrix4_Mul_Model_View)
{
    Base::Float4x4 ViewMatrix
        (
        1.0f, 0.0f, 0.0f,  0.0f,  
        0.0f, 1.0f, 0.0f,  0.0f,  
        0.0f, 0.0f, 1.0f, -8.0f,  
        0.0f, 0.0f, 0.0f,  1.0f
        );

    Base::Float4x4 ModelMatrix
        (
        1.0f, 0.0f, 0.0f, 0.0f,  
        0.0f, 1.0f, 0.0f, 3.06f,  
        0.0f, 0.0f, 1.0f, 0.0f,  
        0.0f, 0.0f, 0.0f, 1.0f
        );

    Base::Float4x4 ModelViewMatrix
        (
        1.0f, 0.0f, 0.0f,  0.0f,  
        0.0f, 1.0f, 0.0f,  3.06f,  
        0.0f, 0.0f, 1.0f, -8.0f,  
        0.0f, 0.0f, 0.0f,  1.0f
        );

    BASE_CHECK(ModelViewMatrix == ViewMatrix * ModelMatrix);
}