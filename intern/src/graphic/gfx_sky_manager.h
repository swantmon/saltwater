
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_sky_facet.h"

namespace Gfx
{
namespace SkyManager
{
    void OnStart();
    void OnExit();

    void Update();

    CSkyFacetPtr CreateSky(unsigned int _FaceSize, const Base::Char* _pFileName);
} // namespace SkyManager
} // namespace Gfx