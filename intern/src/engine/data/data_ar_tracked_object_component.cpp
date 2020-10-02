
#include "engine/engine_precompiled.h"

#include "engine/data/data_ar_tracked_object_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CARTrackedObjectPluginComponent);

    CARTrackedObjectPluginComponent::CARTrackedObjectPluginComponent()
        : m_IsFound      (false)
        , m_UID          (0)
        , m_AppearCounter(0)
    {
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginComponent::~CARTrackedObjectPluginComponent()
    {
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginComponent::SetUID(unsigned int _UID)
    {
        m_UID = _UID;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARTrackedObjectPluginComponent::GetUID() const
    {
        return m_UID;
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginComponent::SetAppearCounter(unsigned int _AppearCounter)
    {
        m_AppearCounter = _AppearCounter;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARTrackedObjectPluginComponent::GetAppearCounter() const
    {
        return m_AppearCounter;
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginComponent::SetIsFound(bool _Flag)
    {
        m_IsFound = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CARTrackedObjectPluginComponent::IsFound() const
    {
        return m_IsFound;
    }
} // namespace Dt