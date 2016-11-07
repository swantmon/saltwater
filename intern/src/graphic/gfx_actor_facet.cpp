
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_actor_facet.h"

namespace Gfx
{
    CModelActorFacet::CModelActorFacet()
        : m_ModelPtr()
    {
        Base::CMemory::Zero(m_MaterialPtrs, sizeof(CMaterialPtr) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CModelActorFacet::~CModelActorFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CModelActorFacet::SetModel(CModelPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CModelPtr CModelActorFacet::GetModel()
    {
        return m_ModelPtr;
    }

    // -----------------------------------------------------------------------------

    void CModelActorFacet::SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_MaterialPtrs[_Surface] = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CModelActorFacet::GetMaterial(unsigned int _Surface)
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

    void CARActorFacet::SetModel(CModelPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CModelPtr CARActorFacet::GetModel()
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