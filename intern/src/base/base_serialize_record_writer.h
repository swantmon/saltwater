#pragma once

#include "base/base_clock.h"
#include "base/base_defines.h"
#include "base/base_serialize_access.h"
#include "base/base_serialize_archive.h"
#include "base/base_serialize_recorder.h"
#include "base/base_timer.h"

namespace SER
{
    class CRecordWriter : public CArchive, public CRecorder
    {
    public:
        enum
        {
            IsWriter = true,
            IsReader = false,
        };

    public:
        typedef std::ostream  CStream;
        typedef CRecordWriter CThis;

    public:
        inline  CRecordWriter(CStream& _rStream, unsigned int _Version);
        inline ~CRecordWriter();

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
        inline void WriteCollection(const TElement* _pElements, unsigned int _NumberOfElements);

        template<typename TElement>
        inline void EndCollection();

        template<typename TElement>
        inline void WritePrimitive(const TElement& _rElement);

        inline void WriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes);

        template<typename TElement>
        inline void WriteClass(const TElement& _rElement);

    public:

        inline CStream* GetStream();

    private:

        CStream* m_pStream;

    public:

        inline void InternWriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes);
    };
} // namespace Core

namespace SER
{
    inline CRecordWriter::CRecordWriter(CStream& _rStream, unsigned int _Version)
        : CArchive (_Version)
        , m_pStream(&_rStream)
    {
        // -----------------------------------------------------------------------------
        // Write header informations (internal format, version)
        // -----------------------------------------------------------------------------
        InternWriteBinary(&m_ArchiveVersion, sizeof(m_ArchiveVersion));
    }

    // -----------------------------------------------------------------------------

    inline CRecordWriter::~CRecordWriter()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CRecordWriter::CThis& CRecordWriter::Write(const TElement& _rElement)
    {
        DispatchWrite(*this, _rElement);

        return *this;
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CRecordWriter::CThis& CRecordWriter::operator << (const TElement& _rElement)
    {
        return Write(_rElement);
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CRecordWriter::CThis& CRecordWriter::operator & (const TElement& _rElement)
    {
        return Write(_rElement);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordWriter::BeginCollection(unsigned int _NumberOfElements)
    {
        WriteBinary(&_NumberOfElements, sizeof(_NumberOfElements));
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordWriter::WriteCollection(const TElement* _pElements, unsigned int _NumberOfElements)
    {
        typedef typename SRemovePointer<TElement>::X XUnqualified;
        bool IsPrimitive = SIsPrimitive<TElement>::Value;

        if (IsPrimitive)
        {
            WriteBinary(_pElements, _NumberOfElements * sizeof(*_pElements));
        }
        else
        {
            for (unsigned int IndexOfElement = 0; IndexOfElement < _NumberOfElements; ++IndexOfElement)
            {
                Write(_pElements[IndexOfElement]);
            }
        }
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordWriter::EndCollection()
    {
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordWriter::WritePrimitive(const TElement& _rElement)
    {
        WriteBinary(&_rElement, sizeof(_rElement));
    }

    // -----------------------------------------------------------------------------

    void CRecordWriter::WriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes)
    {
        // -----------------------------------------------------------------------------
        // Update time
        // -----------------------------------------------------------------------------
        Update();

        double Timecode = GetTime();

        // -----------------------------------------------------------------------------
        // Write time and data to stream
        // -----------------------------------------------------------------------------
        InternWriteBinary(&Timecode, sizeof(Timecode));

        // -----------------------------------------------------------------------------
        // Write data
        // -----------------------------------------------------------------------------
        InternWriteBinary(_pBytes, _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordWriter::WriteClass(const TElement& _rElement)
    {
        SER::Private::CAccess::Write(*this, const_cast<TElement&>(_rElement));
    }

    // -----------------------------------------------------------------------------

    inline CRecordWriter::CStream* CRecordWriter::GetStream()
    {
        return m_pStream;
    }

    // -----------------------------------------------------------------------------

    inline void CRecordWriter::InternWriteBinary(const void* _pBytes, const unsigned int _NumberOfBytes)
    {
        m_pStream->write(static_cast<const char*>(_pBytes), _NumberOfBytes);
    }
} // namespace Core