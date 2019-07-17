
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

		static bool IsEqual(const BInfo& _rLeft, const BInfo& _rRight);
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

	inline bool CTypeInfo::IsEqual(const BInfo& _rLeft, const BInfo& _rRight)
	{
#ifdef PLATFORM_ANDROID
		return std::string(_rLeft.name()) == std::string(_rRight.name());
#else
		return _rLeft == _rRight;
#endif // PLATFORM_ANDROID

	}
} // namespace CORE
