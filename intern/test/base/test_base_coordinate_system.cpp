
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_coordinate_system.h"

BASE_TEST(Test_Base_Coordinate_System)
{
    glm::mat3 In(1.0f);

    glm::mat3 Base1 = Base::CCoordinateSystem::GetBaseMatrix<Base::CCoordinateSystem::Left, Base::CCoordinateSystem::Right>();

    glm::mat3 Base2 = Base::CCoordinateSystem::GetBaseMatrix<Base::CCoordinateSystem::Right, Base::CCoordinateSystem::Left>();

    BASE_CHECK(Base1 == Base2);
}