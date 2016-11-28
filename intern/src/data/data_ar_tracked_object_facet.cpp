
#include "data/data_precompiled.h"

#include "data/data_ar_tracked_object_facet.h"

namespace Dt
{
    CARTrackedObjectPluginFacet::CARTrackedObjectPluginFacet()
        : m_IsFound      (false)
        , m_UID          (0)
        , m_AppearCounter(0)
    {
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginFacet::~CARTrackedObjectPluginFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginFacet::SetUID(unsigned int _UID)
    {
        m_UID = _UID;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARTrackedObjectPluginFacet::GetUID() const
    {
        return m_UID;
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginFacet::SetAppearCounter(unsigned int _AppearCounter)
    {
        m_AppearCounter = _AppearCounter;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARTrackedObjectPluginFacet::GetAppearCounter() const
    {
        return m_AppearCounter;
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginFacet::SetIsFound(bool _Flag)
    {
        m_IsFound = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CARTrackedObjectPluginFacet::IsFound() const
    {
        return m_IsFound;
    }
} // namespace Dt