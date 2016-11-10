
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_actor_facet.h"

namespace Gfx
{
    CMeshActorFacet::CMeshActorFacet()
        : m_ModelPtr()
    {
        Base::CMemory::Zero(m_MaterialPtrs, sizeof(CMaterialPtr) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet::~CMeshActorFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CMeshActorFacet::SetMesh(CMeshPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CMeshActorFacet::GetMesh()
    {
        return m_ModelPtr;
    }

    // -----------------------------------------------------------------------------

    void CMeshActorFacet::SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_MaterialPtrs[_Surface] = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CMeshActorFacet::GetMaterial(unsigned int _Surface)
    {
        return m_MaterialPtrs[_Surface];
    }
} // namespace Gfx

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