
#pragma once

#include "mr/mr_session.h"

namespace MR
{
namespace ControlManager
{
    void OnStart();
    void OnExit();
    void Update();

    void OnPause();
    void OnResume();

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

    void OnDraw();
} // namespace ControlManager
} // namespace MR
