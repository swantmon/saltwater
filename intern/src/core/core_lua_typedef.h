
#pragma once

#include "base/base_typedef.h"

namespace Core
{
namespace Lua
{
    typedef void* BState;
} // namespace Lua
} // namespace Core

namespace Core
{
namespace Lua
{
    typedef int(*FLuaCFunc) (BState _State);
} // namespace Lua
} // namespace Core

namespace Core
{
namespace Lua
{
    struct SValueType
    {
        enum Enum
        {
            Nil,
            Boolean,
            Number,
            String,
            Pointer,
        };
    };
} // namespace Lua
} // namespace Core

namespace Core
{
namespace Lua
{
    struct SLoadFlags
    {
        enum Enum
        {
            DoNotExecute = 0x01,
        };
    };
} // namespace Lua
} // namespace Core