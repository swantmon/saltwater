
#include "plugin_arcore/mr_precompiled.h"

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