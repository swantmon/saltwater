
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_actor_facet.h"

namespace Gfx
{
    CModelActorFacet::CModelActorFacet()
        : m_ModelPtr()
        , m_MaterialPtr()
    {

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

    void CModelActorFacet::SetMaterial(CMaterialPtr _MaterialPtr)
    {
        m_MaterialPtr = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CModelActorFacet::GetMaterial()
    {
        return m_MaterialPtr;
    }
} // namespace Gfx

namespace Gfx
{
    CARActorFacet::CARActorFacet()
        : m_ModelPtr   ()
        , m_MaterialPtr()
    {

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

    void CARActorFacet::SetMaterial(CMaterialPtr _MaterialPtr)
    {
        m_MaterialPtr = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CARActorFacet::GetMaterial()
    {
        return m_MaterialPtr;
    }
} // namespace Gfx