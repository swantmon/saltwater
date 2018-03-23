
#pragma once

#include "mr/mr_camera.h"
#include "mr/mr_light_estimation.h"
#include "mr/mr_marker.h"

namespace MR
{
namespace ControlManager
{
    struct SConfiguration
    {
        enum EDisplayRotation
        {
            Rotation0   = 0,
            Rotation90  = 1,
            Rotation180 = 2,
            Rotation270 = 3,
        };

        void* m_pEnv;
        void* m_pContext;
        void* m_pActivity;

        EDisplayRotation m_Rotation;
        int m_Width;
        int m_Height;
    };
} // namespace ControlManager
} // namespace MR

namespace MR
{
namespace ControlManager
{
    void OnStart(const SConfiguration& _rConfiguration);
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
