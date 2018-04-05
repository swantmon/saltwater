#pragma once

#include "engine/engine_config.h"

#include "core/core_plugin.h"

namespace Core
{
namespace PluginManager
{
    ENGINE_API void LoadPlugin(const char* _pName);

    ENGINE_API bool HasPlugin(const char* _pName);

    ENGINE_API IPlugin& GetPlugin(const char* _pName);
} // namespace PluginManager
} // namespace Core