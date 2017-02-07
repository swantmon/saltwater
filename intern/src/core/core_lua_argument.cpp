
#include "core/core_precompiled.h"

#include "core/core_lua_argument.h"

namespace Core
{
namespace Lua
{
    CArgument::CArgument()
        : m_Type(SValueType::Nil)
        , m_Value()
    {
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(bool _Value)
        : m_Type(SValueType::Boolean)
    {
        m_Value.m_ValueAsBool = _Value;
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(float _Value)
        : m_Type(SValueType::Number)
    {
        m_Value.m_ValueAsDouble = static_cast<double>(_Value);
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(double _Value)
        : m_Type(SValueType::Number)
    {
        m_Value.m_ValueAsDouble = _Value;
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(int _Value)
        : m_Type(SValueType::Number)
    {
        m_Value.m_ValueAsDouble = static_cast<double>(_Value);
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(unsigned int _Value)
        : m_Type(SValueType::Number)
    {
        m_Value.m_ValueAsDouble = static_cast<double>(_Value);
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(const Base::Char* _pValue)
        : m_Type(SValueType::String)
    {
        m_Value.m_pValueAsString = new std::string(_pValue);
    }

    // -----------------------------------------------------------------------------

    CArgument::CArgument(void* const _pValue)
        : m_Type(SValueType::Pointer)
    {
        m_Value.m_pValueAsPtr = _pValue;
    }

    // -----------------------------------------------------------------------------

    SValueType::Enum CArgument::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    bool CArgument::GetBoolean() const
    {
        return m_Value.m_ValueAsBool;
    }

    // -----------------------------------------------------------------------------

    double CArgument::GetNumber() const
    {
        return m_Value.m_ValueAsDouble;
    }

    // -----------------------------------------------------------------------------

    const std::string& CArgument::GetString() const
    {
        return *m_Value.m_pValueAsString;
    }

    // -----------------------------------------------------------------------------

    void* CArgument::GetPointer() const
    {
        return m_Value.m_pValueAsPtr;
    }
} // namespace Lua
} // namespace Core