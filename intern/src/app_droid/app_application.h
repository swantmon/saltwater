
#pragma once

#include "base/base_typedef.h"

// -----------------------------------------------------------------------------
// Forward Declaration
// -----------------------------------------------------------------------------
struct android_app;

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------
namespace App
{
namespace Application
{
    void OnStart(android_app* _pAndroidApp);
    void OnExit();
    void OnRun();

    void ChangeState(unsigned int _State);
} // namespace Application
} // namespace App