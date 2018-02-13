
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_mesh_component.h"

namespace Gfx
{
    CMeshComponent::CMeshComponent()
        : m_ModelPtr()
    {
        Base::CMemory::Zero(m_MaterialPtrs, sizeof(CMaterialPtr) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::~CMeshComponent()
    {
        m_ModelPtr = 0;

        for (unsigned int IndexOfMaterial = 0; IndexOfMaterial < CLOD::s_NumberOfSurfaces; ++IndexOfMaterial)
        {
            m_MaterialPtrs[IndexOfMaterial] = 0;
        }
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

    void CMeshComponent::SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_MaterialPtrs[_Surface] = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CMeshComponent::GetMaterial(unsigned int _Surface)
    {
        return m_MaterialPtrs[_Surface];
    }
} // namespace Gfx