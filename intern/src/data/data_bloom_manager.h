
#pragma once

#include "data/data_bloom_facet.h"

namespace Dt
{
namespace BloomManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CBloomFXFacet* CreateBloomFX();

    void Update();
} // namespace BloomManager
} // namespace Dt
