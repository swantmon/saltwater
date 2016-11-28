
#pragma once

#include "data/data_sky_facet.h"

namespace Dt
{
namespace SkyManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CSkyFacet* CreateSky();

    void Update();
} // namespace SkyManager
} // namespace Dt
