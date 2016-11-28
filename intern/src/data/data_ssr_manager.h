
#pragma once

#include "data/data_ssr_facet.h"

namespace Dt
{
namespace SSRFXManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CSSRFXFacet* CreateSSRFX();

    void Update();
} // namespace SSRFXManager
} // namespace Dt
