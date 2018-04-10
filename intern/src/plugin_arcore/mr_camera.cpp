
#include "plugin_arcore/mr_precompiled.h"

#include "core/core_plugin.h"

#include "plugin_arcore/mr_camera.h"

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

extern "C" CORE_PLUGIN_API_EXPORT MR::CCamera::ETrackingState GetCameraTrackingState(const MR::CCamera* _pCamera)
{
    return _pCamera->GetTackingState();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::mat4 GetCameraViewMatrix(const MR::CCamera* _pCamera)
{
    return _pCamera->GetViewMatrix();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::mat4 GetCameraProjectionMatrix(const MR::CCamera* _pCamera)
{
    return _pCamera->GetProjectionMatrix();
}

extern "C" CORE_PLUGIN_API_EXPORT float GetCameraNear(const MR::CCamera* _pCamera)
{
    return _pCamera->GetNear();
}

extern "C" CORE_PLUGIN_API_EXPORT float GetCameraFar(const MR::CCamera* _pCamera)
{
    return _pCamera->GetFar();
}