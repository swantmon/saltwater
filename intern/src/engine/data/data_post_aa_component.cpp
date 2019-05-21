
#include "engine/engine_precompiled.h"

#include "engine/data/data_post_aa_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CPostAAComponent);

    CPostAAComponent::CPostAAComponent()
        : m_Type(SMAA)
    {
    }

    // -----------------------------------------------------------------------------

    CPostAAComponent::~CPostAAComponent()
    {
    }

    // -----------------------------------------------------------------------------

    void CPostAAComponent::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CPostAAComponent::EType CPostAAComponent::GetType()
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CPostAAComponent::UpdateEffect()
    {
    }
} // namespace Dt