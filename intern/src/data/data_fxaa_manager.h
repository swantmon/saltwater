
#pragma once

#include "data/data_fxaa_facet.h"

namespace Dt
{
namespace FXAAManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CFXAAFXFacet* CreateFXAAFX();

    void Update();
} // namespace FXAAManager
} // namespace Dt
