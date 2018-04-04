
#pragma once

#include "base/base_typedef.h"

#include "core/core_export.h"

namespace Core
{
namespace Time
{
    CORE_API void OnStart();
    CORE_API void OnExit();

    CORE_API void Update();
    
    CORE_API double GetDeltaTimeLastFrame();
    
    CORE_API Base::U64 GetNumberOfFrame();
} // namespace Time
} // namespace Core