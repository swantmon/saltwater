
#pragma once

#include "data/data_light_probe_facet.h"

namespace Dt
{
namespace LightProbeManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CLightProbeFacet* CreateLightProbe();

    void Update();
} // namespace LightProbeManager
} // namespace Dt
