#pragma once

#include "core/core_export.h"

#include <string>

namespace Core
{
namespace AssetManager
{
    CORE_API void SetFilePath(const std::string& _rPath);

    CORE_API const std::string GetPathToFiles();
    CORE_API const std::string GetPathToAssets();
    CORE_API const std::string GetPathToData();
} // namespace AssetManager
} // namespace Core