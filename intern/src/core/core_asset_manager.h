#pragma once

#include "engine/engine_config.h"

#include <string>

namespace Core
{
namespace AssetManager
{
    ENGINE_API void SetFilePath(const std::string& _rPath);

    ENGINE_API const std::string GetPathToFiles();
    ENGINE_API const std::string GetPathToAssets();
    ENGINE_API const std::string GetPathToData();
} // namespace AssetManager
} // namespace Core