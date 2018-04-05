
#include "engine/engine_precompiled.h"

#include "data/data_transformation_facet.h"

namespace Dt
{
    CTransformationFacet::CTransformationFacet()
        : m_WorldMatrix(1.0f)
        , m_Position   (0.0f)
        , m_Rotation   (glm::vec3(0.0f))
        , m_Scale      (1.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CTransformationFacet::~CTransformationFacet()
    {

    }

    // -----------------------------------------------------------------------------


    void CTransformationFacet::SetPosition(const glm::vec3& _rPosition)
    {
        m_Position = _rPosition;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CTransformationFacet::GetPosition()
    {
        return m_Position;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CTransformationFacet::GetPosition() const
    {
        return m_Position;
    }

    // -----------------------------------------------------------------------------

    void CTransformationFacet::SetRotation(const glm::quat& _rRotation)
    {
        m_Rotation = _rRotation;
    }

    // -----------------------------------------------------------------------------

    glm::quat& CTransformationFacet::GetRotation()
    {
        return m_Rotation;
    }

    // -----------------------------------------------------------------------------

    const glm::quat& CTransformationFacet::GetRotation() const
    {
        return m_Rotation;
    }

    // -----------------------------------------------------------------------------

    void CTransformationFacet::SetScale(const glm::vec3& _rScale)
    {
        m_Scale = _rScale;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CTransformationFacet::GetScale()
    {
        return m_Scale;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CTransformationFacet::GetScale() const
    {
        return m_Scale;
    }

    // -----------------------------------------------------------------------------

    void CTransformationFacet::SetWorldMatrix(glm::mat4& _rMatrix)
    {
        m_WorldMatrix = _rMatrix;
    }

    // -----------------------------------------------------------------------------

    glm::mat4& CTransformationFacet::GetWorldMatrix()
    {
        return m_WorldMatrix;
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& CTransformationFacet::GetWorldMatrix() const
    {
        return m_WorldMatrix;
    }
} // namespace Dt