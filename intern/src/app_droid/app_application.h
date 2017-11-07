
#pragma once

#include "base/base_typedef.h"

namespace App
{
namespace Application
{
    void OnStart(android_app* _pAndroidApp);
    void OnExit();
    void OnRun();

    void ChangeState(unsigned int _State);

    const Base::Char* GetAssetPath();
} // namespace Application
} // namespace App