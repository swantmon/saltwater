
#pragma once

#include "base/base_typedef.h"

namespace Core
{
namespace Time
{
    void OnStart();
    void OnExit();

    void Update();
    
    double GetDeltaTimeLastFrame();
    
    Base::U64 GetNumberOfFrame();
} // namespace Time
} // namespace Core