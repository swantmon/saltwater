
#pragma once

#include "base/base_defines.h"
#include "base/base_serialize_archive.h"
#include "base/base_serialize_text_codec.h"

#include <assert.h>
#include <sstream>

namespace SER
{
    class CTextReader : public CArchive
    {
    public:
        enum
        {
            IsWriter = false,
            IsReader = true,
        };

    public:
        using CStream = std::istream;
        typedef CTextReader CThis;

    public:
        inline  CTextReader(CStream& _rStream, unsigned int _Version);
        inline ~CTextReader();

    public:
        template<typename TElement>
        inline CThis& Read(TElement& _rElement);

        template<typename TElement>
        inline CThis& operator >> (TElement& _rElement);

        template<typename TElement>
        inline CThis& operator & (TElement& _rElement);

    public:
        template<typename TElement>
        inline unsigned int BeginCollection();

        template<typename TElement>
        inline void ReadCollection(TElement* _pElements, unsigned int _NumberOfElements);

        template<typename TElement>
        inline void EndCollection();

        template<typename TElement>
        inline void ReadPrimitive(TElement& _rElement);
        
        inline void ReadBinary(void* _pBytes, unsigned int _NumberOfBytes);

        template<typename TElement>
        inline void ReadClass(TElement& _rElement);

    private:
        enum EStatus
        {
            Root,
            Pair,
            List,
            Array,
            Class,
            UndefinedState = -1
        };

    private:
        CStream*     m_pStream;
        unsigned int m_NumberOfIdents;
        EStatus      m_State;

    private:
        template<typename TElement>
        inline unsigned int InternBeginCollection(TElement* _pElements);
        inline unsigned int InternBeginCollection(char* _pElements);
        inline unsigned int InternBeginCollection(wchar_t* _pElements);

        template<typename TElement>
        inline void InternReadCollection(TElement* _pElements, unsigned int _NumberOfElements);
        inline void InternReadCollection(char* _pElements, unsigned int _NumberOfElements);
        inline void InternReadCollection(wchar_t* _pElements, unsigned int _NumberOfElements);

        template<typename TElement>
        inline void InternEndCollection(TElement* _pElements);
        inline void InternEndCollection(char* _pElements);
        inline void InternEndCollection(wchar_t* _pElements);

        template<typename TElement>
        inline void InternReadPrimitive(TElement& _rElement);
        inline void InternReadPrimitive(char& _rElement);
        inline void InternReadPrimitive(wchar_t& _rElement);

        template<typename TElement>
        inline void InternReadValue(TElement& _rElement);

        inline void InternReadChar(const char _Char);
        inline void InternReadChar(const char _Char, unsigned int _NumberOfChars);

        inline void InternReadName(const char* _pChar);

        inline void InternReadEOL();

        inline void InternJumpEOL();

        inline void InternReadIndent();

        inline void InternIgnore(unsigned int _NumberOfBytes);
    };
} // namespace SER

namespace SER
{
    inline CTextReader::CTextReader(CStream& _rStream, unsigned int _Version)
        : CArchive        (_Version)
        , m_pStream       (&_rStream)
        , m_NumberOfIdents(0)
        , m_State         (Root)
    {
        // -----------------------------------------------------------------------------
        // Read header informations (internal format, version)
        // -----------------------------------------------------------------------------        
        InternReadName(Private::Code::s_Version);

        InternReadChar(Private::Code::s_Space);

        InternReadPrimitive(m_ArchiveVersion);

        if (m_ArchiveVersion != _Version)
        {
           throw "Bad resource because of incompatible version.";
        }
    }

    // -----------------------------------------------------------------------------

    inline CTextReader::~CTextReader()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CTextReader::CThis& CTextReader::Read(TElement& _rElement)
    {
        DispatchRead(*this, _rElement);

        return *this;
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CTextReader::CThis& CTextReader::operator >> (TElement& _rElement)
    {
        return Read(_rElement);
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CTextReader::CThis& CTextReader::operator & (TElement& _rElement)
    {
        return Read(_rElement);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline unsigned int CTextReader::BeginCollection()
    {
        using XUnqualified = typename SRemoveQualifier<TElement>::X;

        return InternBeginCollection(static_cast<XUnqualified*>(0));
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::ReadCollection(TElement* _pElements, unsigned int _NumberOfElements)
    {
        InternReadCollection(_pElements, _NumberOfElements);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::EndCollection()
    {
        using XUnqualified = typename SRemoveQualifier<TElement>::X;

        InternEndCollection(static_cast<XUnqualified*>(0));
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::ReadPrimitive(TElement& _rElement)
    {
        InternReadPrimitive(_rElement);
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::ReadBinary(void* _pBytes, unsigned int _NumberOfBytes)
    {
        m_pStream->read(static_cast<char*>(_pBytes), _NumberOfBytes);

        InternReadEOL();
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::ReadClass(TElement& _rElement)
    {
        InternJumpEOL();

        ++ m_NumberOfIdents;

        SER::Private::CAccess::Read(*this, const_cast<TElement&>(_rElement));

        -- m_NumberOfIdents;

        InternReadIndent();

        InternReadChar(Private::Code::s_BracketClose);

        InternReadEOL();
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline unsigned int CTextReader::InternBeginCollection(TElement* _pElements)
    {
        BASE_UNUSED(_pElements);

        int NumberOfElements;

        InternReadIndent();
        InternReadChar(Private::Code::s_BracketOpen);
        InternReadName(Private::Code::s_Collection);

        InternReadChar(Private::Code::s_Space);
        InternReadValue(NumberOfElements);
        InternReadChar(Private::Code::s_Space);

        InternReadEOL();

        m_State = List;
        ++ m_NumberOfIdents;

        return NumberOfElements;
    }

    // -----------------------------------------------------------------------------

    inline unsigned int CTextReader::InternBeginCollection(char* _pElements)
    {
        BASE_UNUSED(_pElements);

        int NumberOfElements;

        InternReadIndent();
        InternReadChar(Private::Code::s_BracketOpen);
        InternReadName(Private::Code::s_Text);

        InternReadChar(Private::Code::s_Space);
        InternReadValue(NumberOfElements);
        InternReadChar(Private::Code::s_Space);

        return NumberOfElements;
    }

    // -----------------------------------------------------------------------------

    inline unsigned int CTextReader::InternBeginCollection(wchar_t* _pElements)
    {
        BASE_UNUSED(_pElements);

        int NumberOfElements;

        InternReadIndent();
        InternReadChar(Private::Code::s_BracketOpen);
        InternReadName(Private::Code::s_Text);

        InternReadChar(Private::Code::s_Space);
        InternReadValue(NumberOfElements);
        InternReadChar(Private::Code::s_Space);

        return NumberOfElements;
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::InternReadCollection(TElement* _pElements, unsigned int _NumberOfElements)
    {
        InternReadIndent();
        InternReadChar(Private::Code::s_BracketOpen);
        InternReadEOL();

        ++ m_NumberOfIdents;

        for (unsigned int IndexOfElement = 0; IndexOfElement < _NumberOfElements; ++IndexOfElement)
        {
            Read(_pElements[IndexOfElement]);
        }

        -- m_NumberOfIdents;

        InternReadIndent();
        InternReadChar(Private::Code::s_BracketClose);
        InternReadEOL();        
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadCollection(char* _pElements, unsigned int _NumberOfElements)
    {
        InternReadChar(Private::Code::s_TextSeperator);

        for (unsigned int IndexOfElement = 0; IndexOfElement < _NumberOfElements; ++IndexOfElement)
        {
            Read(_pElements[IndexOfElement]);
        }

        InternReadChar(Private::Code::s_TextSeperator);
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadCollection(wchar_t* _pElements, unsigned int _NumberOfElements)
    {
        InternReadChar(Private::Code::s_TextSeperator);

        for (unsigned int IndexOfElement = 0; IndexOfElement < _NumberOfElements; ++IndexOfElement)
        {
            Read(_pElements[IndexOfElement]);
        }

        InternReadChar(Private::Code::s_TextSeperator);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::InternEndCollection(TElement* _pElements)
    {
        BASE_UNUSED(_pElements);

        m_State = Root;

        -- m_NumberOfIdents;

        InternReadIndent();
        InternReadChar(Private::Code::s_BracketClose);
        InternReadEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternEndCollection(char* _pElements)
    {
        BASE_UNUSED(_pElements);

        InternReadChar(Private::Code::s_BracketClose);
        InternReadEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternEndCollection(wchar_t* _pElements)
    {
        BASE_UNUSED(_pElements);

        InternReadChar(Private::Code::s_BracketClose);
        InternReadEOL();
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::InternReadPrimitive(TElement& _rElement)
    {
        switch (m_State)
        {
        case Root:
            {
                InternReadIndent();
                (*m_pStream) >> _rElement;
            }
            break;
        case Pair:
            {
                InternReadIndent();
                (*m_pStream) >> _rElement;
                InternReadChar(Private::Code::s_PairSeperator);
            }
            break;
        case List:
            {
                InternReadIndent();
                (*m_pStream) >> _rElement;
                InternReadChar(Private::Code::s_ListSeperator);
            }
            break;
        case Array:
            {
                InternReadIndent();
                (*m_pStream) >> _rElement;
            }
            break;
        case Class:
            {
                InternReadIndent();
                (*m_pStream) >> _rElement;
            }
            break;
        default:
            {
                InternReadIndent();
                (*m_pStream) >> _rElement;
            }
        }

        InternReadEOL();
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadPrimitive(char& _rElement)
    {
        (*m_pStream) >> std::noskipws;
        (*m_pStream) >> _rElement;
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadPrimitive(wchar_t& _rElement)
    {
        BASE_UNUSED(_rElement);

//         (*m_pStream) >> std::noskipws;
//         (*m_pStream) >> _rElement;
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CTextReader::InternReadValue(TElement& _rElement)
    {
        (*m_pStream) >> _rElement;
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadChar(const char _Char)
    {
        BASE_UNUSED(_Char);

        InternIgnore(sizeof(char));
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadChar(const char _Char, unsigned int _NumberOfChars)
    {
        for (unsigned int IndexOfChar = 0; IndexOfChar < _NumberOfChars; ++ IndexOfChar)
        {
            InternReadChar(_Char);
        }
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadName(const char* _pChar)
    {
        InternIgnore(static_cast<unsigned int>(strlen(_pChar)));
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadEOL()
    {
        InternReadChar(Private::Code::s_EOL);
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternJumpEOL()
    {
        m_pStream->ignore(std::numeric_limits<std::streamsize>::max(), Private::Code::s_EOL);
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternReadIndent()
    {
        InternReadChar(Private::Code::s_Indent, m_NumberOfIdents);
    }

    // -----------------------------------------------------------------------------

    inline void CTextReader::InternIgnore(unsigned int _NumberOfBytes)
    {
        m_pStream->ignore(_NumberOfBytes);
    }
} // namespace SER