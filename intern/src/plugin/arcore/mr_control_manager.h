
#pragma once

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_light_estimation.h"
#include "plugin/arcore/mr_marker.h"

#include "graphic/gfx_texture.h"

namespace MR
{
namespace ControlManager
{
    void OnStart();
    void OnExit();
    void Update();

    void OnPause();
    void OnResume();

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

    const CCamera& GetCamera();

    const CLightEstimation& GetLightEstimation();

    const CMarker* AcquireNewMarker(float _X, float _Y);
    void ReleaseMarker(const CMarker* _pMarker);

    Gfx::CTexturePtr GetBackgroundTexture();
} // namespace ControlManager
} // namespace MR
