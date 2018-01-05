//
//  base_exception.h
//  base
//
//  Created by Tobias Schwandt on 11/08/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//


#pragma once

#include "base/base_typedef.h"
#include "base/base_defines.h"

#include <stdarg.h>
#include <string>

#define BASE_THROW()                    { throw CORE::CException(__FILE__, __LINE__); }
#define BASE_THROWC(Code)               { throw CORE::CException(__FILE__, __LINE__, Code); }
#define BASE_THROWM(Text)               { throw CORE::CException(__FILE__, __LINE__, Text); }
#define BASE_THROWCM(Code, Text)        { throw CORE::CException(__FILE__, __LINE__, Code, Text); }
#define BASE_THROWV(Format, ...)        { throw CORE::CException(__FILE__, __LINE__, nullptr, Format, __VA_ARGS__); }
#define BASE_THROWCV(Code, Format, ...) { throw CORE::CException(__FILE__, __LINE__, Code, nullptr, Format, __VA_ARGS__); }

namespace CORE
{
    class CException
    {
    public:
        
        inline CException();
        inline CException(const CException& _rException);
        inline CException(const Char* _pFile, int _Line);
        inline CException(const Char* _pFile, int _Line, int _Code);
        inline CException(const Char* _pFile, int _Line, const Char* _pText);
        inline CException(const Char* _pFile, int _Line, int _Code, const Char* _pText);
        inline CException(const Char* _pFile, int _Line, const Char*, const Char* _pFormat, ...);
        inline CException(const Char* _pFile, int _Line, int _Code, const Char*, const Char* _pFormat, ...);
        inline virtual ~CException();
        
    public:
        
        inline const Char* GetText() const;
        inline const Char* GetFile() const;
        inline int GetLine() const;
        inline int GetCode() const;
        
    private:
        
        static const unsigned int s_MaxNumberOfFormatCharacters = 256;
        
    private:
        
        int         m_Code;
        std::string m_Text;
        std::string m_File;
        int         m_Line;
        
    private:
        
        inline virtual const Char* InternGetText() const;
    };
} // namespace CORE

namespace CORE
{
    inline CException::CException()
        : m_Code(-1)
        , m_Text()
        , m_File()
        , m_Line(-1)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const CException& _rException)
        : m_Code(_rException.m_Code)
        , m_Text(_rException.m_Text)
        , m_File(_rException.m_File)
        , m_Line(_rException.m_Line)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const Char* _pFile, int _Line)
        : m_Code(-1)
        , m_Text()
        , m_File(_pFile)
        , m_Line(_Line)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const Char* _pFile, int _Line, int _Code)
        : m_Code(_Code)
        , m_Text()
        , m_File(_pFile)
        , m_Line(_Line)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const Char* _pFile, int _Line, const Char* _pText)
        : m_Code(-1)
        , m_Text(_pText)
        , m_File(_pFile)
        , m_Line(_Line)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const Char* _pFile, int _Line, int _Code, const Char* _pText)
        : m_Code(_Code)
        , m_Text(_pText)
        , m_File(_pFile)
        , m_Line(_Line)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const Char* _pFile, int _Line, const Char*, const Char* _pFormat, ...)
        : m_Code(-1)
        , m_Text()
        , m_File(_pFile)
        , m_Line(_Line)
    {
        va_list pArguments;
        
        va_start(pArguments, _pFormat);
        
        char Buffer[s_MaxNumberOfFormatCharacters];
        
#if __ANDROID__
        vsnprintf(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#else
        vsnprintf_s(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#endif
        
        m_Text += std::string(Buffer);
        
        va_end(pArguments);
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::CException(const Char* _pFile, int _Line, int _Code, const Char*, const Char* _pFormat, ...)
        : m_Code(_Code)
        , m_Text()
        , m_File(_pFile)
        , m_Line(_Line)
    {
        va_list pArguments;
        
        va_start(pArguments, _pFormat);
        
        char Buffer[s_MaxNumberOfFormatCharacters];
        
#if __ANDROID__
        vsnprintf(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#else
        vsnprintf_s(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#endif
        
        m_Text += std::string(Buffer);
        
        va_end(pArguments);
    }
    
    // -----------------------------------------------------------------------------
    
    inline CException::~CException()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline const Char* CException::GetText() const
    {
        return InternGetText();
    }
    
    // -----------------------------------------------------------------------------
    
    inline const Char* CException::GetFile() const
    {
        return m_File.c_str();
    }
    
    // -----------------------------------------------------------------------------
    
    inline int CException::GetLine() const
    {
        return m_Line;
    }
    
    // -----------------------------------------------------------------------------
    
    inline int CException::GetCode() const
    {
        return m_Code;
    }
    
    // -----------------------------------------------------------------------------
    
    inline const Char* CException::InternGetText() const
    {
        return m_Text.c_str();
    }
    
} // namespace CORE
