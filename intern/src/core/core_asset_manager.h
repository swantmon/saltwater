#pragma once

#include <string>

namespace Core
{
namespace AssetManager
{
    void SetFilePath(const std::string& _rPath);

    const std::string GetPathToFiles();
    const std::string GetPathToAssets();
    const std::string GetPathToData();
} // namespace AssetManager
} // namespace Core