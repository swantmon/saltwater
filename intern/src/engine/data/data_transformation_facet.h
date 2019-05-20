
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

namespace Dt
{
    class ENGINE_API CTransformationFacet
    {
    public:

        void SetPosition(const glm::vec3& _rPosition);
        glm::vec3& GetPosition();
        const glm::vec3& GetPosition() const;

        void SetRotation(const glm::quat& _rRotation);
        glm::quat& GetRotation();
        const glm::quat& GetRotation() const;

        void SetScale(const glm::vec3& _rScale);
        glm::vec3& GetScale();
        const glm::vec3& GetScale() const;

        void SetWorldMatrix(glm::mat4& _rMatrix);
        glm::mat4& GetWorldMatrix();
        const glm::mat4& GetWorldMatrix() const;

    public:

        CTransformationFacet();
        ~CTransformationFacet();

    public:

        template <class TArchive>
        inline void Read(TArchive& _rCodec)
        {
            _rCodec >> m_WorldMatrix[0][0];
            _rCodec >> m_WorldMatrix[0][1];
            _rCodec >> m_WorldMatrix[0][2];
            _rCodec >> m_WorldMatrix[0][3];
            _rCodec >> m_WorldMatrix[1][0];
            _rCodec >> m_WorldMatrix[1][1];
            _rCodec >> m_WorldMatrix[1][2];
            _rCodec >> m_WorldMatrix[1][3];
            _rCodec >> m_WorldMatrix[2][0];
            _rCodec >> m_WorldMatrix[2][1];
            _rCodec >> m_WorldMatrix[2][2];
            _rCodec >> m_WorldMatrix[2][3];
            _rCodec >> m_WorldMatrix[3][0];
            _rCodec >> m_WorldMatrix[3][1];
            _rCodec >> m_WorldMatrix[3][2];
            _rCodec >> m_WorldMatrix[3][3];

            _rCodec >> m_Position[0];
            _rCodec >> m_Position[1];
            _rCodec >> m_Position[2];

            _rCodec >> m_Rotation[0];
            _rCodec >> m_Rotation[1];
            _rCodec >> m_Rotation[2];
            _rCodec >> m_Rotation[3];

            _rCodec >> m_Scale[0];
            _rCodec >> m_Scale[1];
            _rCodec >> m_Scale[2];
        }

        template <class TArchive>
        inline void Write(TArchive& _rCodec)
        {
            _rCodec << m_WorldMatrix[0][0];
            _rCodec << m_WorldMatrix[0][1];
            _rCodec << m_WorldMatrix[0][2];
            _rCodec << m_WorldMatrix[0][3];
            _rCodec << m_WorldMatrix[1][0];
            _rCodec << m_WorldMatrix[1][1];
            _rCodec << m_WorldMatrix[1][2];
            _rCodec << m_WorldMatrix[1][3];
            _rCodec << m_WorldMatrix[2][0];
            _rCodec << m_WorldMatrix[2][1];
            _rCodec << m_WorldMatrix[2][2];
            _rCodec << m_WorldMatrix[2][3];
            _rCodec << m_WorldMatrix[3][0];
            _rCodec << m_WorldMatrix[3][1];
            _rCodec << m_WorldMatrix[3][2];
            _rCodec << m_WorldMatrix[3][3];

            _rCodec << m_Position[0];
            _rCodec << m_Position[1];
            _rCodec << m_Position[2];

            _rCodec << m_Rotation[0];
            _rCodec << m_Rotation[1];
            _rCodec << m_Rotation[2];
            _rCodec << m_Rotation[3];

            _rCodec << m_Scale[0];
            _rCodec << m_Scale[1];
            _rCodec << m_Scale[2];
        }

    private:

        glm::mat4 m_WorldMatrix;           //< Absolute world matrix of the entity inside the map
        glm::vec3 m_Position;              //< Position of the entity to its parent
        glm::quat m_Rotation;              //< Rotation of the entity to its parent
        glm::vec3 m_Scale;                 //< Scale of the entity to its parent

    private:

        friend class CTransformationFacetGUI;
    };
} // namespace Dt

