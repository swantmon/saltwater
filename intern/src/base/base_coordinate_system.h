#pragma once

#include "base/base_typedef.h"
#include "base/base_include_glm.h"

namespace CORE
{
    class CCoordinateSystem
    {
    public:

        enum ECoordinateSystem
        {
            Left,
            Right,
            YUp,
            ZUp,
        };

    public:

        template <ECoordinateSystem SourceSystem, ECoordinateSystem TargetSystem>
        static glm::mat3 GetBaseMatrix();
    };
} // namespace CORE

namespace CORE
{
    template <ECoordinateSystem SourceSystem, ECoordinateSystem TargetSystem>
    static glm::mat3 CCoordinateSystem::GetBaseMatrix()
    {
        assert("Undefined base matrix!");
    }

    // -----------------------------------------------------------------------------

    template <>
    static glm::mat3 CCoordinateSystem::GetBaseMatrix<CCoordinateSystem::ECoordinateSystem::Left, CCoordinateSystem::ECoordinateSystem::Right>()
    {
        return glm::mat3(
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3( 0.0f, 1.0f, 0.0f),
            glm::vec3( 0.0f, 0.0f, 1.0f)
        );
    }

    // -----------------------------------------------------------------------------

    template <>
    static glm::mat3 CCoordinateSystem::GetBaseMatrix<CCoordinateSystem::ECoordinateSystem::Right, CCoordinateSystem::ECoordinateSystem::Left>()
    {
        return glm::mat3(
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3( 0.0f, 1.0f, 0.0f),
            glm::vec3( 0.0f, 0.0f, 1.0f)
        );
    }

    // -----------------------------------------------------------------------------

    template <>
    static glm::mat3 CCoordinateSystem::GetBaseMatrix<CCoordinateSystem::ECoordinateSystem::YUp, CCoordinateSystem::ECoordinateSystem::ZUp>()
    {
        return glm::mat3(
            glm::vec3(1.0f,  0.0f, 0.0f),
            glm::vec3(0.0f,  0.0f, 1.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)
        );
    }

    // -----------------------------------------------------------------------------

    template <>
    static glm::mat3 CCoordinateSystem::GetBaseMatrix<CCoordinateSystem::ECoordinateSystem::ZUp, CCoordinateSystem::ECoordinateSystem::YUp>()
    {
        return glm::mat3(
            glm::vec3(1.0f, 0.0f,  0.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 1.0f,  0.0f)
        );
    }
} // namespace CORE