
#pragma once

#include "base/base_typedef.h"

#include "core/core_lua_typedef.h"

#include <string>

namespace Core
{
namespace Lua
{
    class CArgument
    {
    public:

        CArgument();
        CArgument(bool _Value);
        CArgument(float _Value);
        CArgument(double _Value);
        CArgument(int _Value);
        CArgument(unsigned int _Value);
        CArgument(const Base::Char* _pValue);
        CArgument(void* const _pValue);

    public:

        SValueType::Enum GetType() const;

    public:

        bool GetBoolean() const;
        double GetNumber() const;
        const std::string& GetString() const;
        void* GetPointer() const;

    private:

        union UAny
        {
            bool         m_ValueAsBool;
            double       m_ValueAsDouble;
            std::string* m_pValueAsString;
            void*        m_pValueAsPtr;
        };

    private:

        SValueType::Enum m_Type;
        UAny             m_Value;
    };
} // namespace Lua
} // namespace Core