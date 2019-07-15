
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <typeinfo>

namespace CORE
{
    class CTypeInfo
    {
    public:

        template<class T>
        static CORE::ID GetTypeID();

        template<class T>
        static const char* GetTypeName(const T& _rValue);
    };
} // namespace CORE

namespace CORE
{
    template<class T>
    CORE::ID CTypeInfo::GetTypeID()
    {
        return typeid(T).hash_code();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const char* CTypeInfo::GetTypeName(const T& _rValue)
    {
        return typeid(_rValue).name();
    }
} // namespace CORE
