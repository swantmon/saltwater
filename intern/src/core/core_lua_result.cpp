
#include "core/core_precompiled.h"

#include "core/core_lua_result.h"

#include <assert.h>

namespace Core
{
namespace Lua
{
    CResult::CResult()
        : m_Type (SValueType::Nil)
        , m_Value()
    {
    }

    // -----------------------------------------------------------------------------

    CResult::~CResult()
    {
        if ((m_Type == SValueType::String) && (m_Value.m_pValueAsString != nullptr))
        {
            m_Value.m_pValueAsString->clear();
        }
    }

    // -----------------------------------------------------------------------------

    void CResult::Clear()
    {
        // -----------------------------------------------------------------------------
        // Check if there is remaining string.
        // -----------------------------------------------------------------------------
        if ((m_Type == SValueType::String) && (m_Value.m_pValueAsString != nullptr))
        {
            m_Value.m_pValueAsString->clear();
        }

        m_Value.m_pValueAsString = nullptr;

        // -----------------------------------------------------------------------------
        // Reset type and value.
        // -----------------------------------------------------------------------------
        m_Type                = SValueType::Nil;
        m_Value.m_ValueAsBool = false;
    }

    // -----------------------------------------------------------------------------

    bool CResult::IsEmpty() const
    {
        return (m_Type == SValueType::Nil) && (m_Value.m_ValueAsBool == false);
    }

    // -----------------------------------------------------------------------------

    void CResult::Set(bool _Value)
    {
        assert(IsEmpty());

        m_Type                = SValueType::Boolean;
        m_Value.m_ValueAsBool = _Value;
    }

    // -----------------------------------------------------------------------------

    void CResult::Set(double _Value)
    {
        assert(IsEmpty());

        m_Type                  = SValueType::Number;
        m_Value.m_ValueAsDouble = _Value;
    }

    // -----------------------------------------------------------------------------

    void CResult::Set(const Base::Char* _pValue)
    {
        assert(IsEmpty());

        if (m_Value.m_pValueAsString == nullptr)
        {
            m_Value.m_pValueAsString = new CString();
        }

        m_Type                    = SValueType::String;
        *m_Value.m_pValueAsString = _pValue;
    }

    // -----------------------------------------------------------------------------

    void CResult::Set(void* const _pValue)
    {
        assert(IsEmpty());

        m_Type                = SValueType::Pointer;
        m_Value.m_pValueAsPtr = _pValue;
    }

    // -----------------------------------------------------------------------------

    bool CResult::GetBool() const
    {
        assert((m_Type == SValueType::Boolean) || (m_Type == SValueType::Nil));

        return m_Value.m_ValueAsBool;
    }

    // -----------------------------------------------------------------------------

    double CResult::GetNumber() const
    {
        assert(m_Type == SValueType::Number);

        return m_Value.m_ValueAsDouble;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CResult::GetString() const
    {
        assert(m_Type == SValueType::String && m_Value.m_pValueAsString != nullptr);

        return m_Value.m_pValueAsString->c_str();
    }

    // -----------------------------------------------------------------------------

    void* CResult::GetPointer() const
    {
        assert(m_Type == SValueType::Pointer);

        return m_Value.m_pValueAsPtr;
    }
} // namespace Lua
} // namespace Core