
#pragma once

#include "base/base_typedef.h"

namespace LUA
{
    typedef void* BState;
} // namespace LUA

namespace LUA
{
    typedef int(*FLuaCFunc) (BState _State);
} // namespace LUA

namespace LUA
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
} // namespace LUA

namespace LUA
{
    struct SLoadFlags
    {
        enum Enum
        {
            DoNotExecute = 0x01,
        };
    };
} // namespace LUA