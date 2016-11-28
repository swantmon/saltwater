
#pragma once

#include "data/data_point_light_facet.h"

namespace Dt
{
namespace PointLightManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CPointLightFacet* CreatePointLight();

    void Update();
} // namespace PointLightManager
} // namespace Dt
