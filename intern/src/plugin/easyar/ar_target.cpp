
#include "plugin/easyar/ar_precompiled.h"

#include "engine/core/core_plugin.h"

#include "plugin/easyar/ar_target.h"

namespace AR
{
    CTarget::CTarget()
        : m_TrackingState(Undefined)
        , m_ModelMatrix  (1.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CTarget::~CTarget()
    {

    }

    // -----------------------------------------------------------------------------

    CTarget::ETrackingState CTarget::GetTackingState() const
    {
        return m_TrackingState;
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& CTarget::GetModelMatrix() const
    {
        return m_ModelMatrix;
    }
} // namespace AR

extern "C" CORE_PLUGIN_API_EXPORT AR::CTarget::ETrackingState GetMarkerTrackingState(const AR::CTarget* _pMarker)
{
    return _pMarker->GetTackingState();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::mat4 GetMarkerModelMatrix(const AR::CTarget* _pMarker)
{
    return _pMarker->GetModelMatrix();
}