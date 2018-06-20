#pragma once

#include "engine/engine_config.h"

#include "engine/core/core_plugin.h"

#include <string>

namespace Core
{
namespace PluginManager
{
    ENGINE_API SPluginInfo* LoadPlugin(const std::string& _rLibrary);

    ENGINE_API bool HasPlugin(const std::string& _rName);

    ENGINE_API IPlugin* GetPlugin(const std::string& _rName);

    ENGINE_API void* GetPluginFunction(const std::string& _rName, const std::string& _rFunction);
} // namespace PluginManager
} // namespace Core