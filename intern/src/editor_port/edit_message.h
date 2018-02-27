
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <assert.h>
#include <vector>

namespace Edit
{
    class CMessage
    {
    public:

        CMessage();
        CMessage(bool _Empty);

    public:

        void SetResult(int _Result);
        int GetResult() const;

    public:

        void Reset();
        void Proof();

        Base::Size GetPosition() const;

    public:

        template<typename T>
        void Put(const T _Value);

        template<>
        void Put(const std::string _Value);

        template<typename T>
        const T Get();

        template<>
        const std::string Get();

        template<typename T>
        void Put(Base::Size _Position, const T _Value);

        template<typename T>
        const T Get(Base::Size _Position) const;

        void PutBytes(void* _pBytes, Base::Size _NumberOfBytes);
        Base::Size GetNumberOfBytes() const;
        const void* GetBytes() const;

    private:

        enum EMode
        {
            Write,
            Read,
        };

    private:

        typedef std::vector<Base::U8> CByteVector;
        typedef CByteVector::iterator CIterator;

    private:

        int         m_Result;
        EMode       m_Mode;
        CByteVector m_Bytes;
        CIterator   m_Pos;
    };
} // namespace Edit

namespace Edit
{
    template<typename T>
    void CMessage::Put(const T _Value)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&_Value), reinterpret_cast<const Base::U8*>(&_Value) + sizeof(T));
    }

    // -----------------------------------------------------------------------------

    template<>
    void CMessage::Put(const std::string _Value)
    {
        assert(m_Mode == Write);

        size_t NumberOfChars = _Value.length();

        Put(NumberOfChars);

        if (NumberOfChars > 0)
        {
            m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(_Value.c_str()), reinterpret_cast<const Base::U8*>(_Value.c_str()) + sizeof(Base::Char) * NumberOfChars);
        }
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const T CMessage::Get()
    {
        assert(m_Mode == Read);

        const T Value = *reinterpret_cast<T*>(&(*m_Pos));

        m_Pos += sizeof(T);

        return Value;
    }

    // -----------------------------------------------------------------------------

    template<>
    const std::string CMessage::Get()
    {
        assert(m_Mode == Read);

        size_t NumberOfChars = Get<size_t>();

        if (NumberOfChars == 0) return "";

        std::string ReturnValue(reinterpret_cast<const char*>(&(*m_Pos)), NumberOfChars);

        m_Pos += NumberOfChars;

        return ReturnValue;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CMessage::Put(Base::Size _Position, const T _Value)
    {
        assert(m_Mode == Write);

        *reinterpret_cast<int*>(&m_Bytes[_Position]) = _Value;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const T CMessage::Get(Base::Size _Position) const
    {
        assert(m_Mode == Read);

        const T Value = *reinterpret_cast<const T*>(&m_Bytes[_Position]);

        return Value;
    }
} // namespace Edit
