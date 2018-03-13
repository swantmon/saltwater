
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_coordinate_system.h"

BASE_TEST(Test_Base_Coordinate_System)
{
    auto EpsilonEqual = [&](const glm::mat3& _rLeft, const glm::mat3& _rRight, float _Epsilon) -> bool
    {
        return glm::all(glm::epsilonEqual(_rLeft[0], _rRight[0], _Epsilon))
            && glm::all(glm::epsilonEqual(_rLeft[1], _rRight[1], _Epsilon))
            && glm::all(glm::epsilonEqual(_rLeft[2], _rRight[2], _Epsilon));
    };

    // -----------------------------------------------------------------------------

    glm::vec3 Forward;
    glm::vec3 Right;
    glm::vec3 Up;
    glm::mat3 Base;

    // -----------------------------------------------------------------------------

    Forward = glm::vec3(0.0f, 0.0f, -1.0f);
    Right   = glm::vec3(1.0f, 0.0f,  0.0f);
    Up      = glm::vec3(0.0f, 1.0f,  0.0f);

    Base = Base::CCoordinateSystem::GetBaseMatrix(Right, Up, Forward);

    auto T = glm::mat3(glm::eulerAngleX(glm::radians(-90.0f)));

    BASE_CHECK(EpsilonEqual(Base, glm::mat3(glm::eulerAngleX(glm::radians(-90.0f))), 0.01f));

    // -----------------------------------------------------------------------------

    Forward = glm::vec3(0.0f,  0.0f, 1.0f);
    Right   = glm::vec3(1.0f,  0.0f, 0.0f);
    Up      = glm::vec3(0.0f, -1.0f, 0.0f);

    Base = Base::CCoordinateSystem::GetBaseMatrix(Right, Up, Forward);

    BASE_CHECK(EpsilonEqual(Base, glm::mat3(glm::eulerAngleX(glm::radians(90.0f))), 0.01f));

    // -----------------------------------------------------------------------------

    Forward = glm::vec3(0.0f, -1.0f,  0.0f);
    Right   = glm::vec3(1.0f,  0.0f,  0.0f);
    Up      = glm::vec3(0.0f,  0.0f, -1.0f);

    Base = Base::CCoordinateSystem::GetBaseMatrix(Right, Up, Forward);

    BASE_CHECK(EpsilonEqual(Base, glm::mat3(glm::eulerAngleX(glm::radians(180.0f))), 0.01f));
}