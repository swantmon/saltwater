#pragma once

#include "mr/mr_camera.h"

namespace MR
{
namespace CameraManager
{
    void OnStart();

    void OnExit();

    void Update();

    const CCamera& GetCamera();
} // namespace CameraManager
} // namespace MR