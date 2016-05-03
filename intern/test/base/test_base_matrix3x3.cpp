
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_matrix3x3.h"


BASE_TEST(Test_Base_Matrix3_Add)
{
    Base::Float3x3 Matrix1
    (
     1,1,1,
     2,2,2,
     3,3,3
     );
    
    Base::Float3x3 Matrix2
    (
     4,4,4,
     3,3,3,
     2,2,2
     );
    
    Base::Float3x3 MatrixResult
    (
     5,5,5,
     5,5,5,
     5,5,5
     );
    
    BASE_CHECK(MatrixResult == Matrix2 + Matrix1);
    BASE_CHECK(MatrixResult == Matrix1 + Matrix2);
}

BASE_TEST(Test_Base_Matrix3_Mul)
{
    Base::Float3x3 Matrix1
    (
     1,2,3,
     1,2,3,
     1,2,3
    );
    
    Base::Float3x3 Matrix2
    (
     4,3,2,
     4,3,2,
     4,3,2
    );
    
    Base::Float3x3 MatrixResult
    (
     24,18,12,
     24,18,12,
     24,18,12
    );
    
    Base::Float3x3 MatrixResultInverse
    (
     9,18,27,
     9,18,27,
     9,18,27
    );
    
    BASE_CHECK(MatrixResult        == Matrix1 * Matrix2);
    BASE_CHECK(MatrixResult        != Matrix2 * Matrix1);
    BASE_CHECK(MatrixResultInverse == Matrix2 * Matrix1);
}

BASE_TEST(Test_Base_Matrix3_Inverse)
{
    Base::Float3x3 Matrix
    (
     1,1,1,
     2,1,0,
     1,0,1
    );
    
    Base::Float3x3 MatrixInverse
    (
     -0.5, 0.5, 0.5,
      1  , 0  , -1,
      0.5,-0.5, 0.5
    );
    
    BASE_CHECK(MatrixInverse                               == Matrix.GetInverted());
    BASE_CHECK(MatrixInverse.GetInverted() * MatrixInverse == Base::Float3x3::s_Identity);
    BASE_CHECK(MatrixInverse * MatrixInverse.GetInverted() == Base::Float3x3::s_Identity);
    BASE_CHECK(MatrixInverse                               == Matrix.Invert());
}