
#pragma once

#include "mr/mr_camera.h"
#include "mr/mr_light_estimation.h"
#include "mr/mr_marker.h"

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

    void OnDraw();

    const CCamera& GetCamera();

    const CLightEstimation& GetLightEstimation();

    const CMarker* AcquireNewMarker(float _X, float _Y);
    void ReleaseMarker(const CMarker* _pMarker);
} // namespace ControlManager
} // namespace MR
