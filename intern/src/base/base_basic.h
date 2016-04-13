
#pragma once

#include "base/base_defines.h"

#include <stddef.h>

namespace CORE
{
namespace Private
{
namespace Native
{
    typedef signed char        S8;                                                  ///< 8 bit signed integer
    typedef unsigned char      U8;                                                  ///< 8 bit unsigned integer
    typedef signed short       S16;                                                 ///< 16 bit signed integer
    typedef unsigned short     U16;                                                 ///< 16 bit unsigned integer
    typedef signed long        S32;                                                 ///< 32 bit signed integer
    typedef unsigned long      U32;                                                 ///< 32 bit unsigned integer
    typedef signed long long   S64;                                                 ///< 64 bit signed integer
    typedef unsigned long long U64;                                                 ///< 64 bit unsigned integer
    typedef float              F32;                                                 ///< 32 bit floating point
    typedef double             F64;                                                 ///< 64 bit floating point
    typedef unsigned int       Word;                                                ///< Represents the data bus size
    typedef void*              Ptr;                                                 ///< Typeless pointer type
    typedef char               Char;                                                ///< Character type
    typedef wchar_t            WChar;                                               ///< Wide character type
    typedef char               C8;                                                  ///< 8 bit character type
    typedef wchar_t            C16;                                                 ///< 16 bit character type
    typedef unsigned int       C32;                                                 ///< 32 bit character type
    typedef U32                Time32;                                              ///< 32 bit time
    typedef U64                Time64;                                              ///< 64 bit time
    typedef size_t             Size;                                                ///< Portable unsigned integral size type
#if __APPLE__
    typedef decltype(nullptr)  NullPtr;                                             ///< Null pointer type
#else
    typedef nullptr_t          NullPtr;                                             ///< Null pointer type
#endif
} // namespace Native
} // namespace Private
} // namespace CORE