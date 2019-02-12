#pragma once

#include "engine/engine_config.h"

#include "engine/core/core_plugin.h"

#include <string>

namespace Core
{
namespace PluginManager
{
    ENGINE_API void OnStart();

    ENGINE_API void Update();

    ENGINE_API void OnPause();

    ENGINE_API void OnResume();

    ENGINE_API void OnExit();

    ENGINE_API void SetLibraryPath(const std::string& _rPath);
    
    ENGINE_API bool LoadPlugin(const std::string& _rName);

    ENGINE_API void* GetPluginFunction(const std::string& _rName, const std::string& _rFunction);
} // namespace PluginManager
} // namespace Core