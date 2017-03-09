
#pragma once

#include "data/data_area_light_facet.h"

namespace Dt
{
namespace AreaLightManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CAreaLightFacet* CreateAreaLight();

    void Update();
} // namespace AreaLightManager
} // namespace Dt
