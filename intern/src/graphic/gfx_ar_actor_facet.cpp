
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_ar_actor_facet.h"

namespace Gfx
{
    CARActorFacet::CARActorFacet()
        : m_ModelPtr()
    {
        Base::CMemory::Zero(m_MaterialPtrs, sizeof(CMaterialPtr) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CARActorFacet::~CARActorFacet()
    {
        m_ModelPtr = 0;

        for (unsigned int IndexOfMaterial = 0; IndexOfMaterial < CLOD::s_NumberOfSurfaces; ++IndexOfMaterial)
        {
            m_MaterialPtrs[IndexOfMaterial] = 0;
        }
    }

    // -----------------------------------------------------------------------------

    void CARActorFacet::SetMesh(CMeshPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CARActorFacet::GetMesh()
    {
        return m_ModelPtr;
    }

    // -----------------------------------------------------------------------------

    void CARActorFacet::SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_MaterialPtrs[_Surface] = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CARActorFacet::GetMaterial(unsigned int _Surface)
    {
        return m_MaterialPtrs[_Surface];
    }
} // namespace Gfx