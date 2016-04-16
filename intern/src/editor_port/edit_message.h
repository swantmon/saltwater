
#pragma once

#include "base/base_typedef.h"

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

        void PutBool(bool _Value);
        bool GetBool();
        void PutBool(Base::Size _Position, bool _Value);
        bool GetBool(Base::Size _Position) const;

        void PutInt(int _Value);
        int GetInt();
        void PutInt(Base::Size _Position, int _Value);
        int GetInt(Base::Size _Position) const;

        void PutLongLong(long long _Value);
        long long GetLongLong();
        void PutLongLong(Base::Size _Position, long long _Value);
        long long GetLongLong(Base::Size _Position) const;

        void PutFloat(float _Value);
        float GetFloat();
        void PutFloat(Base::Size _Position, float _Value);
        float GetFloat(Base::Size _Position) const;

        void PutDouble(double _Value);
        double GetDouble();
        void PutDouble(Base::Size _Position, double _Value);
        double GetDouble(Base::Size _Position) const;

        void PutString(const Base::Char* _pString);
        Base::Char* GetString(Base::Char* _pString, Base::Size _MaxNumberOfChars);

        void PutAddress(void* _pPointer);
        void* GetAddress();

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

    private:

        template <Base::Size TNumberOfBytes>
        void InternPutAddress(void* _pPointer);
        template <>
        void InternPutAddress<4>(void* _pPointer);
        template <>
        void InternPutAddress<8>(void* _pPointer);

        template <Base::Size TNumberOfBytes>
        void* InternGetAddress();
        template <>
        void* InternGetAddress<4>();
        template <>
        void* InternGetAddress<8>();
    };
} // namespace Edit

namespace Edit
{
    template <typename T>
    T CMessage::GetEnum()
    {
        return static_cast<T>(GetInt());
    }
} // namespace Edit
