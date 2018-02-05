
#pragma once

#include "base/base_lib_glm.h"

namespace Dt
{
    class CTransformationFacet
    {
    public:

        void SetPosition(const glm::vec3& _rPosition);
        glm::vec3& GetPosition();
        const glm::vec3& GetPosition() const;

        void SetRotation(const glm::vec3& _rRotation);
        glm::vec3& GetRotation();
        const glm::vec3& GetRotation() const;

        void SetScale(const glm::vec3& _rScale);
        glm::vec3& GetScale();
        const glm::vec3& GetScale() const;

        void SetWorldMatrix(glm::mat4& _rMatrix);
        glm::mat4& GetWorldMatrix();
        const glm::mat4& GetWorldMatrix() const;

    public:

        CTransformationFacet();
        ~CTransformationFacet();

    private:

        glm::mat4 m_WorldMatrix;           //< Absolute world matrix of the entity inside the map
        glm::vec3 m_Position;              //< Position of the entity to its parent
        glm::vec3 m_Rotation;              //< Rotation of the entity to its parent
        glm::vec3 m_Scale;                 //< Scale of the entity to its parent
    };
} // namespace Dt

