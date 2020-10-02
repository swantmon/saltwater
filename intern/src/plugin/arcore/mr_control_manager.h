
#pragma once

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_light_estimation.h"
#include "plugin/arcore/mr_marker.h"

#include "engine/graphic/gfx_texture.h"

#include <arcore_c_api.h>

namespace MR
{
namespace ControlManager
{
    struct SSettings
    {
        float m_Near;
        float m_Far;
    };
} // namespace ControlManager
} // namespace MR

namespace MR
{
namespace ControlManager
{
    void OnStart();
    void OnExit();
    void Update();

    void OnPause();
    void OnResume();

    void UpdateBackground();

    const CCamera& GetCamera();

    const CLightEstimation& GetLightEstimation();

    const CMarker* AcquireNewMarker(float _X, float _Y);
    void ReleaseMarker(const CMarker* _pMarker);

    Gfx::CTexturePtr GetBackgroundTexture();

    const SSettings& GetSettings();
    void SetSettings(const SSettings& _rSettings);
    void ResetSettings();

    ArSession* GetCurrentSession();
    ArFrame* GetCurrentFrame();
} // namespace ControlManager
} // namespace MR
