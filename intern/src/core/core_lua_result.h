
#pragma once

#include "core/core_lua_typedef.h"

#include <string>

namespace Core
{
namespace Lua
{
    class CResult
    {
    public:

        CResult();
        ~CResult();

    public:

        void Clear();
        bool IsEmpty() const;

    public:

        bool GetBool() const;
        double GetNumber() const;
        const Base::Char* GetString() const;
        void* GetPointer() const;

    public:

        void Set(bool _Value);
        void Set(double _Value);
        void Set(const Base::Char* _pValue);
        void Set(void* const _pValue);

    private:

        typedef std::string CString;

    private:

        union UValue
        {
            bool     m_ValueAsBool;
            double   m_ValueAsDouble;
            CString* m_pValueAsString;
            void*    m_pValueAsPtr;
        };

    private:

        SValueType::Enum m_Type;
        UValue           m_Value;
    };
} // namespace Lua
} // namespace Core