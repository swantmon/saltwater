
#pragma once

namespace Dt
{
    struct SFXType
    {
        enum Enum
        {
            Bloom,
            SSR,
            DOF,
            PostAA,
            SSAO,
            VolumeFog,
            UndefinedType = -1,
        };
    };
} // namespace Dt