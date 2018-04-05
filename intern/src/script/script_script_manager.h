
#pragma once

#include "engine/engine_config.h"

namespace Scpt
{
namespace ScriptManager
{
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();
    ENGINE_API void Update();

    ENGINE_API void OnPause();
    ENGINE_API void OnResume();
} // namespace ScriptManager
} // namespace Scpt