
#pragma once

#include "base/base_string.h"
#include "base/base_typedef.h"

#include "core/core_lua_typedef.h"

namespace LUA
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
        const Base::Char* GetString() const;
        void* GetPointer() const;

    private:

        typedef Base::CharString CString;

    private:

        union UAny
        {
            bool		m_ValueAsBool;
            double		m_ValueAsDouble;
            CString*	m_pValueAsString;
            void*       m_pValueAsPtr;
        };

    private:

        SValueType::Enum m_Type;
        UAny             m_Value;
    };
} // namespace LUA