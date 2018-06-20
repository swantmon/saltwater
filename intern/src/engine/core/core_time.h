
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

namespace Core
{
namespace Time
{
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();

    ENGINE_API void Update();
    
    ENGINE_API double GetDeltaTimeLastFrame();

    ENGINE_API double GetTime();
    
    ENGINE_API Base::Size GetNumberOfFrame();
} // namespace Time
} // namespace Core