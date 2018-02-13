
#pragma once

#include "base/base_basic.h"
#include "base/base_defines.h"

namespace CORE
{
    typedef Private::Native::S8         S8;                                                         ///< 8 bit signed integer
    typedef Private::Native::U8         U8;                                                         ///< 8 bit unsigned integer
    typedef Private::Native::S16        S16;                                                        ///< 16 bit signed integer
    typedef Private::Native::U16        U16;                                                        ///< 16 bit unsigned integer
    typedef Private::Native::S32        S32;                                                        ///< 32 bit signed integer
    typedef Private::Native::U32        U32;                                                        ///< 32 bit unsigned integer
    typedef Private::Native::S64        S64;                                                        ///< 64 bit signed integer
    typedef Private::Native::U64        U64;                                                        ///< 64 bit unsigned integer
    typedef Private::Native::F32        F32;                                                        ///< 32 bit floating point
    typedef Private::Native::F64        F64;                                                        ///< 64 bit floating point
    typedef Private::Native::Word       Word;                                                       ///< Represents the data bus size
    typedef Private::Native::Ptr        Ptr;                                                        ///< Typeless pointer type
    typedef Private::Native::NullPtr    NullPtr;                                                    ///< Null pointer
    typedef Private::Native::Char       Char;                                                       ///< Character type
    typedef Private::Native::WChar      WChar;                                                      ///< Wide character type
    typedef Private::Native::C8         C8;                                                         ///< 8 bit character type
    typedef Private::Native::C16        C16;                                                        ///< 16 bit character type
    typedef Private::Native::C32        C32;                                                        ///< 32 bit character type
    typedef Private::Native::Size       Size;                                                       ///< Portable unsigned integral size type
    typedef Private::Native::ID         ID;                                                         ///< ID to identify objects and classes
    typedef Private::Native::Time32     Time32;                                                     ///< 32 bit time
    typedef Private::Native::Time64     Time64;                                                     ///< 64 bit time
} // namespace CORE

namespace CORE
{
    typedef unsigned int BHash;
} // namespace CORE