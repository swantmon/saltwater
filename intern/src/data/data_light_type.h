
#pragma once

namespace Dt
{
    struct SLightType
    {
        enum Enum
        {
            Point,
            Sun,
            LightProbe,
            Sky,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt