
#pragma once

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
    int ConvertGenerationFlags(int _Flags);
} // namespace AssetImporter
} // namespace Core