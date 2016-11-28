#pragma once

namespace Dt
{
    struct SPluginType
    {
        enum Enum
        {
            ARControlManager,
            ARTrackedObject,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt