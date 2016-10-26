
#include "editor_port/edit_precompiled.h"

#include "editor_port/edit_message.h"

#include <assert.h>

namespace Edit
{
    CMessage::CMessage()
        : m_Result(0)
        , m_Mode  (Write)
        , m_Bytes ()
        , m_Pos   (m_Bytes.begin())
    {
    }
    
    // -----------------------------------------------------------------------------

    CMessage::CMessage(bool _Empty)
        : m_Result(0)
        , m_Mode (Write)
        , m_Bytes()
        , m_Pos  (m_Bytes.begin())
    {
        if (_Empty)
        {
            const Base::Char Terminator = '\0';

            m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&Terminator), reinterpret_cast<const Base::U8*>(&Terminator) + sizeof(Terminator));
        }
    }

    // -----------------------------------------------------------------------------

    void CMessage::Reset()
    {
        m_Pos = m_Bytes.begin();
    }

    // -----------------------------------------------------------------------------

    void CMessage::Proof()
    {
        m_Mode = Read;
    }

    // -----------------------------------------------------------------------------

    void CMessage::SetResult(int _Result)
    {
        m_Result = _Result;
    }

    // -----------------------------------------------------------------------------

    int CMessage::GetResult() const
    {
        return m_Result;
    }

    // -----------------------------------------------------------------------------

    Base::Size CMessage::GetPosition() const
    {
        return m_Pos - const_cast<CByteVector&>(m_Bytes).begin();
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutBool(bool _Value)
    {
        Base::U8 Value;

        Value = _Value ? 1 : 0;

        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&Value), reinterpret_cast<const Base::U8*>(&Value) + sizeof(Value));
    }

    // -----------------------------------------------------------------------------

    bool CMessage::GetBool()
    {
        Base::U8 Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<Base::U8*>(&(*m_Pos));

        m_Pos += sizeof(Value);

        return Value == 1;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutBool(Base::Size _Position, bool _Value)
    {
        Base::U8 Value;

        Value = _Value ? 1 : 0;

        assert(m_Mode == Write);

        *reinterpret_cast<Base::U8*>(&m_Bytes[_Position]) = _Value;
    }

    // -----------------------------------------------------------------------------

    bool CMessage::GetBool(Base::Size _Position) const
    {
        Base::U8 Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<const Base::U8*>(&m_Bytes[_Position]);

        return Value == 1;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutInt(int _Value)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&_Value), reinterpret_cast<const Base::U8*>(&_Value) + sizeof(int));
    }

    // -----------------------------------------------------------------------------

    int CMessage::GetInt()
    {
        int Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<int*>(&(*m_Pos));

        m_Pos += sizeof(Value);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutInt(Base::Size _Position, int _Value)
    {
        assert(m_Mode == Write);

        *reinterpret_cast<int*>(&m_Bytes[_Position]) = _Value;
    }

    // -----------------------------------------------------------------------------

    int CMessage::GetInt(Base::Size _Position) const
    {
        int Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<const int*>(&m_Bytes[_Position]);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutLongLong(long long _Value)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&_Value), reinterpret_cast<const Base::U8*>(&_Value) + sizeof(_Value));
    }

    // -----------------------------------------------------------------------------

    long long CMessage::GetLongLong()
    {
        long long Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<long long*>(&(*m_Pos));

        m_Pos += sizeof(Value);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutLongLong(Base::Size _Position, long long _Value)
    {
        assert(m_Mode == Write);

        *reinterpret_cast<long long*>(&m_Bytes[_Position]) = _Value;
    }

    // -----------------------------------------------------------------------------

    long long CMessage::GetLongLong(Base::Size _Position) const
    {
        long long Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<const long long*>(&m_Bytes[_Position]);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutFloat(float _Value)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&_Value), reinterpret_cast<const Base::U8*>(&_Value) + sizeof(_Value));
    }

    // -----------------------------------------------------------------------------

    float CMessage::GetFloat()
    {
        float Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<float*>(&(*m_Pos));

        m_Pos += sizeof(Value);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutFloat(Base::Size _Position, float _Value)
    {
        assert(m_Mode == Write);

        *reinterpret_cast<float*>(&m_Bytes[_Position]) = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMessage::GetFloat(Base::Size _Position) const
    {
        float Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<const float*>(&m_Bytes[_Position]);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutDouble(double _Value)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&_Value), reinterpret_cast<const Base::U8*>(&_Value) + sizeof(_Value));
    }

    // -----------------------------------------------------------------------------

    double CMessage::GetDouble()
    {
        double Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<double*>(&(*m_Pos));

        m_Pos += sizeof(Value);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutDouble(Base::Size _Position, double _Value)
    {
        assert(m_Mode == Write);

        *reinterpret_cast<double*>(&m_Bytes[_Position]) = _Value;
    }

    // -----------------------------------------------------------------------------

    double CMessage::GetDouble(Base::Size _Position) const
    {
        double Value;

        assert(m_Mode == Read);

        Value = *reinterpret_cast<const double*>(&m_Bytes[_Position]);

        return Value;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutString(const Base::Char* _pString)
    {
        Base::Size NumberOfChars;

        assert(m_Mode == Write);
        assert(_pString != 0);

        NumberOfChars = strlen(_pString);

        PutInt(static_cast<int>(NumberOfChars));

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(_pString), reinterpret_cast<const Base::U8*>(_pString) + sizeof(Base::Char) * NumberOfChars);
    }

    // -----------------------------------------------------------------------------

    Base::Char* CMessage::GetString(Base::Char* _pString, Base::Size _MaxNumberOfChars)
    {
        Base::Size NumberOfChars;

        BASE_UNUSED(_MaxNumberOfChars);

        assert(m_Mode == Read);

        NumberOfChars = GetInt();

        assert(NumberOfChars < _MaxNumberOfChars);

        ::memcpy(_pString, &(*m_Pos), NumberOfChars);

        _pString[NumberOfChars] = '\0';

        m_Pos += NumberOfChars;

        return _pString;
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutAddress(void* _pPointer)
    {
        InternPutAddress<sizeof(_pPointer)>(_pPointer);
    }

    // -----------------------------------------------------------------------------

    template <>
    void CMessage::InternPutAddress<4>(void* _pPointer)
    {
        PutInt(static_cast<int>((Base::Size) _pPointer));
    }

    // -----------------------------------------------------------------------------

    template <>
    void CMessage::InternPutAddress<8>(void* _pPointer)
    {
        PutLongLong((long long) _pPointer);
    }

    // -----------------------------------------------------------------------------

    void* CMessage::GetAddress()
    {
        return InternGetAddress<sizeof(void*)>();
    }

    // -----------------------------------------------------------------------------

    template <>
    void* CMessage::InternGetAddress<4>()
    {
        return (void*) static_cast<Base::Size>(GetInt());
    }

    // -----------------------------------------------------------------------------

    template <>
    void* CMessage::InternGetAddress<8>()
    {
        return (void*) GetLongLong();
    }

    // -----------------------------------------------------------------------------

    void CMessage::PutBytes(void* _pBytes, Base::Size _NumberOfBytes)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), static_cast<Base::U8*>(_pBytes), static_cast<Base::U8*>(_pBytes) + _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    Base::Size CMessage::GetNumberOfBytes() const
    {
        return m_Bytes.size();
    }

    // -----------------------------------------------------------------------------

    const void* CMessage::GetBytes() const
    {
        return &m_Bytes.front();
    }
} // namespace Edit
