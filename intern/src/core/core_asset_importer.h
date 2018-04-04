
#pragma once

#include "core/core_asset_generator.h"
#include "core/core_export.h"

#include <string>

namespace Core
{
namespace AssetImporter
{
    CORE_API const void* AllocateAssimpImporter(const std::string& _rFile, int _GeneratorFlag);

    CORE_API const void* AllocateTinyXMLImporter(const std::string& _rFile);

    CORE_API void* GetNativeAccessFromImporter(const void* _pImporter);

    CORE_API void ReleaseImporter(const void* _pImporter);
} // namespace AssetImporter
} // namespace Core