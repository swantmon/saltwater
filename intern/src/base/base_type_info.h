
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

		// -----------------------------------------------------------------------------
		// This function is only necessary on Android because th hash-code can be
		// different between different shared libraries. The name stays the same.
		// TODO: Find out why this behavior happens and how to solve it more
		//       efficiently.
		// -----------------------------------------------------------------------------
		static bool IsEqualName(const BInfo& _rLeft, const BInfo& _rRight);
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

	// -----------------------------------------------------------------------------

	inline bool CTypeInfo::IsEqualName(const BInfo& _rLeft, const BInfo& _rRight)
	{
		return std::string(_rLeft.name()) == std::string(_rRight.name());
	}
} // namespace CORE
