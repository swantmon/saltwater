
////////////////////////////////////////////////////////////////////////////////
///
/// \file base_serialize_binary_writer.h
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
#include "base/base_serialize_archive.h"

#include <assert.h>
#include <sstream>
#include <fstream>

namespace SER
{
	class CBinaryWriter : public CArchive
    {
    public:
        enum
        {
            IsWriter = true,
            IsReader = false,
        };

    public:
        typedef std::ostream   CStream;
        typedef CBinaryWriter  CThis;

    public:
        inline  CBinaryWriter(CStream& _rStream, unsigned int _Version);
        inline ~CBinaryWriter();

    public:
        template<typename TElement>
        inline CThis& Write(const TElement& _rElement);

        template<typename TElement>
        inline CThis& operator << (const TElement& _rElement);

        template<typename TElement>
        inline CThis& Read(TElement& _rElement) { return *this; };

        template<typename TElement>
        inline CThis& operator >> (TElement& _rElement) { return *this; };

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
        
        virtual void WriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes) override;

        template<typename TElement>
        inline void WriteClass(const TElement& _rElement);

        template<typename TElement>
        inline void ReadPrimitive(TElement& _rElement) {};

        inline void ReadBinary(void* _pBytes, unsigned int _NumberOfBytes) {};

        template<typename TElement>
        inline void ReadClass(TElement& _rElement) {};

    private:
        CStream*     m_pStream;
        unsigned int m_NumberOfElements;
    };
} // namespace SER

namespace SER
{
	inline CBinaryWriter::CBinaryWriter(CStream& _rStream, unsigned int _Version)
        : CArchive          (_Version)
        , m_pStream         (&_rStream)
        , m_NumberOfElements(0)
    {
        // -----------------------------------------------------------------------------
        // Write header informations (internal format, version)
        // -----------------------------------------------------------------------------
        WriteBinary(&m_ArchiveVersion, sizeof(m_ArchiveVersion));
    }

    // -----------------------------------------------------------------------------

    inline CBinaryWriter::~CBinaryWriter()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CBinaryWriter::CThis& CBinaryWriter::Write(const TElement& _rElement)
    {
        DispatchWrite(*this, _rElement);

        return *this;
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CBinaryWriter::CThis& CBinaryWriter::operator << (const TElement& _rElement)
    {
        return Write(_rElement);
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CBinaryWriter::CThis& CBinaryWriter::operator & (const TElement& _rElement)
    {
        return Write(_rElement);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryWriter::BeginCollection(unsigned int _NumberOfElements)
    {
        m_NumberOfElements = _NumberOfElements;

        WriteBinary(&m_NumberOfElements, sizeof(m_NumberOfElements));
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryWriter::WriteCollection(const TElement* _pElements)
    {
        typedef typename SRemovePointer<TElement>::X XUnqualified;
        bool IsPrimitive = SIsPrimitive<TElement>::Value;

        if (IsPrimitive)
        {
            WriteBinary(_pElements, m_NumberOfElements * sizeof(*_pElements));
        }
        else
        {
            unsigned int IndexOfElement;
            unsigned int NumberOfElements;

            NumberOfElements = m_NumberOfElements;

            for (IndexOfElement = 0; IndexOfElement < NumberOfElements; ++IndexOfElement)
            {
                Write(_pElements[IndexOfElement]);
            }
        }
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryWriter::EndCollection()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryWriter::WritePrimitive(const TElement& _rElement)
    {
        WriteBinary(&_rElement, sizeof(_rElement));
    }

    // -----------------------------------------------------------------------------

    void CBinaryWriter::WriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes)
    {
        m_pStream->write(static_cast<const char*>(_pBytes), _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryWriter::WriteClass(const TElement& _rElement)
    {
        SER::Private::CAccess::Write(*this, const_cast<TElement&>(_rElement));
    }
} // namespace SER