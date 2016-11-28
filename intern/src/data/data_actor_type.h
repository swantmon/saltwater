
#pragma once

namespace Dt
{
    struct SActorType
    {
        enum Enum
        {
            Node,
            Mesh,
            AR,
            Camera,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt