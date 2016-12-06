
#pragma once

#include "data/data_post_aa_facet.h"

namespace Dt
{
namespace PostAAManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CPostAAFXFacet* CreatePostAAFX();

    void Update();
} // namespace PostAAManager
} // namespace Dt
