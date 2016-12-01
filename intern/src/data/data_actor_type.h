
#pragma once

namespace Dt
{
    struct SActorType
    {
        enum Enum
        {
            Node,
            Mesh,
            Camera,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt