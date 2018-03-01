
#pragma once

namespace Core
{
namespace AssetGenerator
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
} // namespace AssetGenerator
} // namespace Core