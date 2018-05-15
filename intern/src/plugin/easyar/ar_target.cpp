
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

extern "C" CORE_PLUGIN_API_EXPORT AR::CTarget::ETrackingState GetTargetTrackingState(const AR::CTarget* _pTarget)
{
    return _pTarget->GetTackingState();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::mat4 GetTargetModelMatrix(const AR::CTarget* _pTarget)
{
    return _pTarget->GetModelMatrix();
}