
#pragma once

#include "core/core_asset_generator.h"

#include <string>

namespace Core
{
namespace AssetImporter
{
    const void* AllocateAssimpImporter(const std::string& _rFile, int _GeneratorFlag);

    const void* AllocateTinyXMLImporter(const std::string& _rFile);

    void* GetNativeAccessFromImporter(const void* _pImporter);

    void ReleaseImporter(const void* _pImporter);
} // namespace AssetImporter
} // namespace Core