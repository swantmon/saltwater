
#pragma once

#include <string>

namespace Core
{
namespace AssetImporter
{
    struct SGeneratorFlag
    {
        enum
        {
            Nothing      = 0x00,
            Default      = 0x01,
            FlipUVs      = 0x02,
            RealtimeFast = 0x04
        };
    };
} // namespace AssetImporter
} // namespace Core

namespace Core
{
namespace AssetImporter
{
    const void* AllocateAssimpImporter(const std::string& _rFile, int _GeneratorFlag);

    const void* AllocateTinyXMLImporter(const std::string& _rFile);

    void* GetNativeAccessFromImporter(const void* _pImporter);

    void ReleaseImporter(const void* _pImporter);

    int ConvertGenerationFlags(int _Flags);
} // namespace AssetImporter
} // namespace Core