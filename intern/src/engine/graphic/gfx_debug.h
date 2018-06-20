#pragma once

#include "engine/engine_config.h"

namespace Gfx
{
namespace Debug
{
    void OnStart();
    void OnExit();

    ENGINE_API void Push(int _ID);
    ENGINE_API void Pop();
} // namespace Debug
} // namespace Gfx