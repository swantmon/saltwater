
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <typeinfo>
#include <typeindex>

namespace CORE
{
    class CTypeInfo
    {
	public:

		using BInfo = std::type_index;

    public:

		template<class T>
		static BInfo Get();

        template<class T>
        static BInfo Get(const T& _rValue);
    };
} // namespace CORE

namespace CORE
{
	template<class T>
	CTypeInfo::BInfo CTypeInfo::Get()
	{
		return std::type_index(typeid(T));
	}

    // -----------------------------------------------------------------------------

    template<class T>
	CTypeInfo::BInfo CTypeInfo::Get(const T& _rValue)
    {
        return std::type_index(typeid(_rValue));
    }
} // namespace CORE
