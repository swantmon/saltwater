
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

        template<typename T>
        const T Get();

        template<typename T>
        void Put(Base::Size _Position, const T _Value);

        template<typename T>
        const T Get(Base::Size _Position) const;

        void PutString(const Base::Char* _pString);
        Base::Char* GetString(Base::Char* _pString, Base::Size _MaxNumberOfChars);

        void PutBytes(void* _pBytes, Base::Size _NumberOfBytes);
        Base::Size GetNumberOfBytes() const;
        const void* GetBytes() const;

        template <typename T>
        T GetEnum();


    private:

        enum EMode
        {
            Write,
            Read,
        };

    private:

        typedef std::vector<Base::U8>  CByteVector;
        typedef CByteVector::iterator  CIterator;

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

    template<typename T>
    const T CMessage::Get()
    {
        assert(m_Mode == Read);

        const T Value = *reinterpret_cast<T*>(&(*m_Pos));

        m_Pos += sizeof(T);

        return Value;
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

    // -----------------------------------------------------------------------------

    template <typename T>
    T CMessage::GetEnum()
    {
        return static_cast<T>(GetInt());
    }
} // namespace Edit
