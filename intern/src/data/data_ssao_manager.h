
#pragma once

#include "data/data_ssao_facet.h"

namespace Dt
{
namespace SSAOManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CSSAOFXFacet* CreateSSAOFX();

    void Update();
} // namespace SSAOManager
} // namespace Dt
