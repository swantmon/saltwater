
#pragma once

#include "data/data_volume_fog_facet.h"

namespace Dt
{
namespace VolumeFogManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CVolumeFogFXFacet* CreateVolumeFogFX();

    void Update();
} // namespace VolumeFogManager
} // namespace Dt
