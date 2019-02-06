#pragma once

#include "engine/engine_config.h"

#include "engine/core/core_plugin.h"

#include <string>

namespace Core
{
namespace PluginManager
{
	ENGINE_API void Start();

	ENGINE_API void Update();

	ENGINE_API void Pause();

	ENGINE_API void Resume();

	ENGINE_API void Exit();

	ENGINE_API void SetLibraryPath(const std::string& _rPath);

    ENGINE_API bool HasPlugin(const std::string& _rName);

    ENGINE_API IPlugin* GetPlugin(const std::string& _rName);

    ENGINE_API void* GetPluginFunction(const std::string& _rName, const std::string& _rFunction);
} // namespace PluginManager
} // namespace Core