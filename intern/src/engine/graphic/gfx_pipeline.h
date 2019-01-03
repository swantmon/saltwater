
#pragma once

#include "engine/engine_config.h"

namespace Gfx
{
namespace Pipeline
{
    ENGINE_API void OnStart();

    ENGINE_API void OnExit();

    ENGINE_API void Render();

    ENGINE_API unsigned int RegisterWindow(void* _pWindow, int _VSync = 0);

    ENGINE_API void ActivateWindow(unsigned int _WindowID);

    ENGINE_API void InitializeWindow(unsigned int _WindowID, void* _pWindow, int _VSync = 0);

    ENGINE_API void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);
} // namespace Engine
} // namespace Gfx