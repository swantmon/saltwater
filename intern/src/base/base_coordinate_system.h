#pragma once

#include "base/base_typedef.h"
#include "base/base_include_glm.h"

namespace CORE
{
    class CCoordinateSystem
    {
    public:

        inline static glm::mat3 GetBaseMatrix(const glm::vec3& _rRight, const glm::vec3& _rUp, const glm::vec3& _rForword);
    };
} // namespace CORE

namespace CORE
{
    inline glm::mat3 CCoordinateSystem::GetBaseMatrix(const glm::vec3& _rRight, const glm::vec3& _rUp, const glm::vec3& _rForword)
    {
        static const glm::mat3 s_Reference(
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // -----------------------------------------------------------------------------
        // Idea from:
        // https://gamedev.stackexchange.com/questions/26084/how-to-get-the-rotation-matrix-to-transform-between-two-3d-cartesian-coordinate
        // -----------------------------------------------------------------------------
        glm::mat3 Target(
            _rRight,
            _rUp,
            _rForword
        );

        return glm::inverse(Target * glm::inverse(s_Reference));
    }
} // namespace CORE