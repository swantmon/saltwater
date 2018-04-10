
#include "plugin_arcore/mr_precompiled.h"

#include "core/core_plugin.h"

#include "plugin_arcore/mr_marker.h"

namespace MR
{
    CMarker::CMarker()
        : m_TrackingState(Undefined)
        , m_ModelMatrix  (1.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CMarker::~CMarker()
    {

    }

    // -----------------------------------------------------------------------------

    CMarker::ETrackingState CMarker::GetTackingState() const
    {
        return m_TrackingState;
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& CMarker::GetModelMatrix() const
    {
        return m_ModelMatrix;
    }
} // namespace MR

extern "C" CORE_PLUGIN_API_EXPORT MR::CMarker::ETrackingState GetMarkerTrackingState(const MR::CMarker* _pMarker)
{
    return _pMarker->GetTackingState();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::mat4 GetMarkerModelMatrix(const MR::CMarker* _pMarker)
{
    return _pMarker->GetModelMatrix();
}