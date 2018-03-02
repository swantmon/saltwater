
#pragma once

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

    android_app* GetAndroidApp();
} // namespace Application
} // namespace App