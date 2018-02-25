
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_mesh_component.h"

namespace Gfx
{
    CMeshComponent::CMeshComponent()
        : m_ModelPtr   ()
        , m_MaterialPtr()
    {
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::~CMeshComponent()
    {
        m_ModelPtr    = 0;
        m_MaterialPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMesh(CMeshPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CMeshComponent::GetMesh()
    {
        return m_ModelPtr;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMaterial(CMaterialPtr _MaterialPtr)
    {
        m_MaterialPtr = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CMeshComponent::GetMaterial()
    {
        return m_MaterialPtr;
    }
} // namespace Gfx