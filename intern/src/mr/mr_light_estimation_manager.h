
#pragma once

#include "mr/mr_light_estimation.h"

namespace MR
{
namespace LightEstimationManager
{
    void OnStart();

    void OnExit();

    void Update();

    const CLightEstimation& GetLightEstimation();
} // namespace LightEstimationManager
} // namespace MR