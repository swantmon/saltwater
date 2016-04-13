
#include "data/data_transformation_facet.h"

namespace Dt
{
    CTransformationFacet::CTransformationFacet()
        : m_WorldMatrix()
        , m_Position   (Base::Float3::s_Zero)
        , m_Rotation   (Base::Float3::s_Zero)
        , m_Scale      (Base::Float3::s_One)
    {

    }

    // -----------------------------------------------------------------------------

    CTransformationFacet::~CTransformationFacet()
    {

    }

    // -----------------------------------------------------------------------------


    void CTransformationFacet::SetPosition(const Base::Float3& _rPosition)
    {
        m_Position = _rPosition;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CTransformationFacet::GetPosition()
    {
        return m_Position;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CTransformationFacet::GetPosition() const
    {
        return m_Position;
    }

    // -----------------------------------------------------------------------------

    void CTransformationFacet::SetRotation(const Base::Float3& _rRotation)
    {
        m_Rotation = _rRotation;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CTransformationFacet::GetRotation()
    {
        return m_Rotation;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CTransformationFacet::GetRotation() const
    {
        return m_Rotation;
    }

    // -----------------------------------------------------------------------------

    void CTransformationFacet::SetScale(const Base::Float3& _rScale)
    {
        m_Scale = _rScale;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CTransformationFacet::GetScale()
    {
        return m_Scale;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CTransformationFacet::GetScale() const
    {
        return m_Scale;
    }

    // -----------------------------------------------------------------------------

    void CTransformationFacet::SetWorldMatrix(Base::Float4x4& _rMatrix)
    {
        m_WorldMatrix = _rMatrix;
    }

    // -----------------------------------------------------------------------------

    Base::Float4x4& CTransformationFacet::GetWorldMatrix()
    {
        return m_WorldMatrix;
    }

    // -----------------------------------------------------------------------------

    const Base::Float4x4& CTransformationFacet::GetWorldMatrix() const
    {
        return m_WorldMatrix;
    }
} // namespace Dt