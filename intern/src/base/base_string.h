
//
//  base_string.h
//  base
//
//  Created by Tobias Schwandt on 19/04/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"
#include "base/base_memory.h"
#include "base/base_typedef.h"

namespace STR
{
    template<typename T = Base::Char>
    class CString
    {

    public:

        typedef CString<T> CThis;

    public:

        CString();
        CString(const T* _pString);
        CString(const CThis& _rCopy);
        ~CString();

    public:

        inline CThis& operator = (const CThis& _rRight);

    public:

        inline T& operator [] (const int _Index);
        inline const T& operator [] (const int _Index) const;

    public:

        inline bool operator == (const CThis& _rRight) const;
        inline bool operator == (const T* _pRight) const;

        inline bool operator != (const CThis& _rRight) const;
        inline bool operator != (const T* _pRight) const;

    public:

        friend bool operator == (const T* _pLeft, const CThis& _rRight)
        {
            return strcmp(_rRight.m_pString, _pLeft) == 0;
        }

        friend bool operator != (const T* _pLeft, const CThis& _rRight)
        {
            return strcmp(_rRight.m_pString, _pLeft) != 0;
        }

    public:

        inline void Set(const T* _pString);
        inline void Set(const CThis& _rCopy);
        inline void Clear();

    public:

        inline const T* GetConst() const;

    public:

        Base::Size GetLength() const;

    private:

        T*         m_pString;
        Base::Size m_Length;
    };
} // namespace STR

namespace STR
{
    typedef CString<Base::Char>  CharString;
    typedef CString<Base::WChar> WCharString;
} // namespace STR

namespace STR
{
    template<typename T>
    CString<T>::CString()
        : m_pString(0)
        , m_Length (0)
    {

    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CString<T>::CString(const T* _pString)
        : m_pString(0)
        , m_Length (0)
    {
        Set(_pString);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CString<T>::CString(const CThis& _rCopy)
        : m_pString(0)
        , m_Length (0)
    {
        Set(_rCopy);
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    CString<T>::~CString()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    typename CString<T>::CThis& CString<T>::operator = (const CThis& _rRight)
    {
        Set(_rRight); 

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    T& CString<T>::operator [] (const int _Index)
    {
        assert(_Index < m_Length && _Index >= 0);

        return m_pString[_Index];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const T& CString<T>::operator [] (const int _Index) const
    {
        assert(_Index < m_Length && _Index >= 0);

        return m_pString[_Index];
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CString<T>::operator == (const CThis& _rRight) const
    {
        return strcmp(m_pString, _rRight.m_pString) == 0;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CString<T>::operator == (const T* _pRight) const
    {
        return strcmp(m_pString, _pRight) == 0;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CString<T>::operator != (const CThis& _rRight) const
    {
        return strcmp(m_pString, _rRight.m_pString) != 0;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    bool CString<T>::operator != (const T* _pRight) const
    {
        return strcmp(m_pString, _pRight)  != 0;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CString<T>::Set(const T* _pString)
    {
        Clear();

        if (_pString != 0)
        {
            Base::Size LengthOfString = strlen(_pString);

            m_Length = LengthOfString;

            Base::Size NumberOfBytes = (LengthOfString + 1) * sizeof(Base::Char);

            m_pString = static_cast<Base::Char*>(Base::CMemory::Allocate(NumberOfBytes));

            strcpy_s(m_pString, NumberOfBytes, _pString);
        }
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CString<T>::Set(const CThis& _rCopy)
    {
        Clear();

        if (_rCopy.m_pString != 0)
        {
            m_Length = _rCopy.m_Length;

            Base::Size NumberOfBytes = (_rCopy.m_Length + 1) * sizeof(Base::Char);

            m_pString = static_cast<Base::Char*>(Base::CMemory::Allocate(NumberOfBytes));

            strcpy_s(m_pString, NumberOfBytes, _rCopy.m_pString);
        }
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CString<T>::Clear()
    {
        if (m_pString != 0)
        {
            Base::CMemory::Free(m_pString);

            m_pString = 0;
            m_Length  = 0;
        }
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    const T* CString<T>::GetConst() const
    {
        return m_pString;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    Base::Size CString<T>::GetLength() const
    {
        return m_Length;
    }
} // namespace STR