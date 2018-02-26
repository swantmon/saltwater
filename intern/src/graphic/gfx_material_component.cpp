
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_material_component.h"

namespace Gfx
{
    CMaterialComponent::CMaterialComponent()
        : m_MaterialPtr()
    {
    }

    // -----------------------------------------------------------------------------

    CMaterialComponent::~CMaterialComponent()
    {
        m_MaterialPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetMaterial(CMaterialPtr _MaterialPtr)
    {
        m_MaterialPtr = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CMaterialComponent::GetMaterial()
    {
        return m_MaterialPtr;
    }
} // namespace Gfx