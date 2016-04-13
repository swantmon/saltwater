
////////////////////////////////////////////////////////////////////////////////
///
/// \file base_serialize_text_writer.h
///
/// \author Tobias Schwandt
/// \author Credits to Joerg Sahm
/// \author Copyright (c) Tobias Schwandt. All rights reserved.
///
/// \date 2012-2013
///
/// \version 1.0
/// 
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base/base_defines.h"
#include "base/base_serialize_access.h"
#include "base/base_serialize_archive.h"
#include "base/base_serialize_text_codec.h"

#include <assert.h>
#include <sstream>
#include <fstream>

namespace SER
{
	class CTextWriter : public CArchive
    {
    public:
        enum
        {
            IsWriter = true,
            IsReader = false,
        };

    public:
        typedef std::ostream CStream;
        typedef CTextWriter  CThis;

    public:
        inline  CTextWriter(CStream& _rStream, unsigned int _Version);
        inline ~CTextWriter();

    public:
        template<typename TElement>
        inline CThis& Write(const TElement& _rElement);

        template<typename TElement>
        inline CThis& operator << (const TElement& _rElement);

        template<typename TElement>
        inline CThis& operator & (const TElement& _rElement);

    public:
        template<typename TElement>
        inline void BeginCollection(unsigned int _NumberOfElements);

        template<typename TElement>
        inline void WriteCollection(const TElement* _pElements);

        template<typename TElement>
        inline void EndCollection();
        
        template<typename TElement>
        inline void WritePrimitive(const TElement& _rElement);
                
        inline void WriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes);

        template<typename TElement>
        inline void WriteClass(const TElement& _rElement);

    private:
        enum EStatus
        {
            Default,
            List,
            UndefinedState = -1
        };

    private:
        CStream*     m_pStream;
        unsigned int m_NumberOfElements;
        unsigned int m_NumberOfIdents;
        EStatus      m_State;

    private:
        template<typename TElement>
        inline void InternBeginCollection(const TElement* _pElements);
        inline void InternBeginCollection(const char* _pElements);
        inline void InternBeginCollection(const wchar_t* _pElements);

        template<typename TElement>
        inline void InternWriteCollection(const TElement* _pElements);
        inline void InternWriteCollection(const char* _pElements);
        inline void InternWriteCollection(const wchar_t* _pElements);

        template<typename TElement>
        inline void InternEndCollection(const TElement* _pElements);
        inline void InternEndCollection(const char* _pElements);
        inline void InternEndCollection(const wchar_t* _pElements);

        template<typename TElement>
        inline void InternWritePrimitive(const TElement& _rElement);
        inline void InternWritePrimitive(const char& _rElement);
        inline void InternWritePrimitive(const wchar_t& _rElement);

        template<typename TElement>
        inline void InternWriteValue(const TElement& _rElement);

        inline void InternWriteChar(const char _Char);
        inline void InternWriteChar(const char _Char, unsigned int _NumberOfChars);

        inline void InternWriteName(const char* _pChar);

        inline void InternWriteEOL();

        inline void InternWriteIndent();
    };
} // namespace SER

namespace SER
{
	inline CTextWriter::CTextWriter(CStream& _rStream, unsigned int _Version)
        : CArchive          (_Version)
        , m_pStream         (&_rStream)
        , m_NumberOfElements(0)
        , m_NumberOfIdents  (0)
        , m_State           (Default)
    {
        // -----------------------------------------------------------------------------
        // Write header informations (internal format, version)
        // -----------------------------------------------------------------------------
        InternWriteName(Private::Code::s_Version);

        InternWriteChar(Private::Code::s_Space);
        
        InternWritePrimitive(m_ArchiveVersion);
    }

    // -----------------------------------------------------------------------------

    inline CTextWriter::~CTextWriter()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CTextWriter::CThis& CTextWriter::Write(const TElement& _rElement)
    {
        DispatchWrite(*this, _rElement);

        return *this;
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CTextWriter::CThis& CTextWriter::operator << (const TElement& _rElement)
    {
        return Write(_rElement);
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CTextWriter::CThis& CTextWriter::operator & (const TElement& _rElement)
    {
        return Write(_rElement);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::BeginCollection(unsigned int _NumberOfElements)
    {
        typedef typename SRemoveQualifier<TElement>::X XUnqualified;

        m_NumberOfElements = _NumberOfElements;

        InternBeginCollection(static_cast<XUnqualified*>(0));
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::WriteCollection(const TElement* _pElements)
    {
        InternWriteCollection(_pElements);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::EndCollection()
    {
        typedef typename SRemoveQualifier<TElement>::X XUnqualified;

        InternEndCollection(static_cast<XUnqualified*>(0));
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::WritePrimitive(const TElement& _rElement)
    {
        InternWritePrimitive(_rElement);
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::WriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes)
    {
        m_pStream->write(static_cast<const char*>(_pBytes), _NumberOfBytes);

        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::WriteClass(const TElement& _rElement)
    {
        typedef typename SRemoveQualifier<TElement>::X XUnqualified;

        static const unsigned int s_MaxLengthOfClassName = 2048;

        char ClassName[s_MaxLengthOfClassName];

        strcpy(ClassName, typeid( XUnqualified ).name());

        InternWriteIndent();

        InternWriteChar(Private::Code::s_BracketOpen);

        InternWriteName(ClassName);

        InternWriteEOL();

        ++ m_NumberOfIdents;

        SER::Private::CAccess::Serialize(*this, const_cast<TElement&>(_rElement));

        -- m_NumberOfIdents;

        InternWriteIndent();

        InternWriteChar(Private::Code::s_BracketClose);

        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::InternBeginCollection(const TElement* _pElements)
    {
        InternWriteIndent();
        InternWriteChar(Private::Code::s_BracketOpen);
        InternWriteName(Private::Code::s_Collection);

        InternWriteChar(Private::Code::s_Space);
        InternWriteValue(m_NumberOfElements);
        InternWriteChar(Private::Code::s_Space);

        InternWriteEOL();

        m_State = List;
        ++ m_NumberOfIdents;
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternBeginCollection(const char* _pElements)
    {
        BASE_UNUSED(_pElements);

        InternWriteIndent();
        InternWriteChar(Private::Code::s_BracketOpen);
        InternWriteName(Private::Code::s_Text);

        InternWriteChar(Private::Code::s_Space);
        InternWriteValue(m_NumberOfElements);
        InternWriteChar(Private::Code::s_Space);
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternBeginCollection(const wchar_t* _pElements)
    {
        BASE_UNUSED(_pElements);

        InternWriteIndent();
        InternWriteChar(Private::Code::s_BracketOpen);
        InternWriteName(Private::Code::s_Text);

        InternWriteChar(Private::Code::s_Space);
        InternWriteValue(m_NumberOfElements);
        InternWriteChar(Private::Code::s_Space);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::InternWriteCollection(const TElement* _pElements)
    {
        unsigned int IndexOfElement;
        unsigned int NumberOfElements;

        NumberOfElements = m_NumberOfElements;

        InternWriteIndent();
        InternWriteChar(Private::Code::s_BracketOpen);
        InternWriteEOL();

        ++ m_NumberOfIdents;

        for (IndexOfElement = 0; IndexOfElement < NumberOfElements; ++IndexOfElement)
        {
            Write(_pElements[IndexOfElement]);
        }

        -- m_NumberOfIdents;

        InternWriteIndent();
        InternWriteChar(Private::Code::s_BracketClose);
        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWriteCollection(const char* _pElements)
    {
        unsigned int IndexOfElement;
        unsigned int NumberOfElements;

        NumberOfElements = m_NumberOfElements;

        InternWriteChar(Private::Code::s_TextSeperator);

        for (IndexOfElement = 0; IndexOfElement < NumberOfElements; ++IndexOfElement)
        {
            Write(_pElements[IndexOfElement]);
        }

        InternWriteChar(Private::Code::s_TextSeperator);
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWriteCollection(const wchar_t* _pElements)
    {
        unsigned int IndexOfElement;
        unsigned int NumberOfElements;

        NumberOfElements = m_NumberOfElements;

        InternWriteChar(Private::Code::s_TextSeperator);

        for (IndexOfElement = 0; IndexOfElement < NumberOfElements; ++IndexOfElement)
        {
            Write(_pElements[IndexOfElement]);
        }

        InternWriteChar(Private::Code::s_TextSeperator);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::InternEndCollection(const TElement* _pElements)
    {
        m_State = Default;

        -- m_NumberOfIdents;

        InternWriteIndent();
        InternWriteChar(Private::Code::s_BracketClose);
        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternEndCollection(const char* _pElements)
    {
        BASE_UNUSED(_pElements);

        InternWriteChar(Private::Code::s_BracketClose);
        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternEndCollection(const wchar_t* _pElements)
    {
        BASE_UNUSED(_pElements);

        InternWriteChar(Private::Code::s_BracketClose);
        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::InternWritePrimitive(const TElement& _rElement)
    {
        switch (m_State)
        {
        case Default:
            {
                InternWriteIndent();
                (*m_pStream) << _rElement;
            }
            break;
        case List:
            {
                InternWriteIndent();
                (*m_pStream) << _rElement;
                InternWriteChar(Private::Code::s_ListSeperator);
            }
            break;
        default:
            {
                InternWriteIndent();
                (*m_pStream) << _rElement;
            }
        }
        
        InternWriteEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWritePrimitive(const char& _rElement)
    {
        (*m_pStream) << _rElement;
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWritePrimitive(const wchar_t& _rElement)
    {
        (*m_pStream) << _rElement;
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextWriter::InternWriteValue(const TElement& _rElement)
    {
        (*m_pStream) << _rElement;
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWriteChar(const char _Char)
    {
        (*m_pStream) << _Char;
    }

    // -----------------------------------------------------------------------------
   
    inline void CTextWriter::InternWriteChar(const char _Char, unsigned int _NumberOfChars)
    {
        for (unsigned int IndexOfChar = 0; IndexOfChar < _NumberOfChars; ++ IndexOfChar)
        {
            InternWriteChar(_Char);
        }
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWriteName(const char* _pChar)
    {
        (*m_pStream) << _pChar;
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWriteEOL()
    {
        InternWriteChar(Private::Code::s_EOL);
    }

    // -----------------------------------------------------------------------------

    inline void CTextWriter::InternWriteIndent()
    {
        InternWriteChar(Private::Code::s_Indent, m_NumberOfIdents);
    }
} // namespace SER