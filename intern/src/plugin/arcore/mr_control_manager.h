
#pragma once

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_light_estimation.h"
#include "plugin/arcore/mr_marker.h"

#include "engine/graphic/gfx_texture.h"


namespace MR
{
namespace ControlManager
{
    struct SSettings
    {
        bool m_ShowPlanes;
        bool m_ShowPoints;
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

    const CCamera& GetCamera();

    const CLightEstimation& GetLightEstimation();

    const CMarker* AcquireNewMarker(float _X, float _Y);
    void ReleaseMarker(const CMarker* _pMarker);

    Gfx::CTexturePtr GetBackgroundTexture();

    const SSettings& GetSettings();
    void SetSettings(const SSettings& _rSettings);
    void ResetSettings();
} // namespace ControlManager
} // namespace MR
