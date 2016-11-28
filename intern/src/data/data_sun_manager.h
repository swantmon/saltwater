
#pragma once

#include "data/data_sun_facet.h"

namespace Dt
{
namespace SunManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CSunLightFacet* CreateSunLight();

    void Update();
} // namespace SunManager
} // namespace Dt
