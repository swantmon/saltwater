
#include "mr/mr_precompiled.h"

#include "mr/mr_camera.h"

namespace MR
{
    CCamera::CCamera()
        : m_TrackingState   (Undefined)
        , m_ViewMatrix      (1.0f)
        , m_ProjectionMatrix(1.0f)
        , m_Near            (0.1f)
        , m_Far             (10.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CCamera::~CCamera()
    {

    }

    // -----------------------------------------------------------------------------

    CCamera::ETrackingState CCamera::GetTackingState() const
    {
        return m_TrackingState;
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& CCamera::GetViewMatrix() const
    {
        return m_ViewMatrix;
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& CCamera::GetProjectionMatrix() const
    {
        return m_ProjectionMatrix;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetNear() const
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetFar() const
    {
        return m_Far;
    }
} // namespace MR