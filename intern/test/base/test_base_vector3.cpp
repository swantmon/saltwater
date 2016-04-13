
#include "base/base_test_defines.h"

#include "base/base_vector3.h"

BASE_TEST(Test_Base_Vector3_Add)
{
    Base::Float3 Vector1(1, 2, 3);

    Base::Float3 Vector2(4, 3, 2);

    Base::Float3 VectorResult(5, 5, 5);

    BASE_CHECK(VectorResult == Vector2 + Vector1);
    BASE_CHECK(VectorResult == Vector1 + Vector2);
}

BASE_TEST(Test_Base_Vector3_Mul)
{
    Base::Float3 Vector1(1, 2, 3);

    Base::Float3 Vector2(4, 3, 2);

    Base::Float3 VectorResult(4, 6, 6);

    BASE_CHECK(VectorResult == Vector2 * Vector1);
    BASE_CHECK(VectorResult == Vector1 * Vector2);
}

BASE_TEST(Test_Base_Vector3_Length)
{
    Base::Float3 Vector1(1, 2, 3);

    BASE_CHECK(3.74165750f == Vector1.Length());
}

BASE_TEST(Test_Base_Vector3_Cross)
{
    Base::Float3 Vector1(1, 2, 3);

    Base::Float3 Vector2(0, 1, 0);

    Base::Float3 VectorResult(-3, 0, 1);

    BASE_CHECK(VectorResult == Vector1.CrossProduct(Vector2));
}