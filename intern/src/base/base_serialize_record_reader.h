#pragma once

#include "base/base_clock.h"
#include "base/base_defines.h"
#include "base/base_memory.h"
#include "base/base_serialize_access.h"
#include "base/base_serialize_archive.h"
#include "base/base_serialize_recorder.h"
#include "base/base_timer.h"

using namespace Base;

namespace SER
{
    class CRecordReader : public CArchive, public CRecorder
    {
    public:
        enum
        {
            IsWriter = false,
            IsReader = true,
        };

    public:
        typedef std::istream  CStream;
        typedef CRecordReader CThis;

    public:
        inline  CRecordReader(CStream& _rStream, unsigned int _Version);
        inline ~CRecordReader();

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

    public:

        CStream* GetStream();

        inline double PeekTimecode();

        inline void SkipTime();

        inline bool IsEnd() const;

    private:

        CStream* m_pStream;
        double   m_Timecode;

    private:

        inline void InternReadBinary(void* _pBytes, unsigned int _NumberOfBytes);
    };
} // namespace SER

namespace SER
{
    inline CRecordReader::CRecordReader(CStream& _rStream, unsigned int _Version)
        : CArchive (_Version)
        , CRecorder( )
        , m_pStream(&_rStream)
    {
        // -----------------------------------------------------------------------------
        // Read header informations (internal format, version)
        // -----------------------------------------------------------------------------        
        InternReadBinary(&m_ArchiveVersion, sizeof(m_ArchiveVersion));

        if (m_ArchiveVersion != _Version)
        {
            throw "Bad resource because of incompatible version.";
        }
    }

    // -----------------------------------------------------------------------------

    inline CRecordReader::~CRecordReader()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CRecordReader::CThis& CRecordReader::Read(TElement& _rElement)
    {
        DispatchRead(*this, _rElement);

        return *this;
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CRecordReader::CThis& CRecordReader::operator >> (TElement& _rElement)
    {
        return Read(_rElement);
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CRecordReader::CThis& CRecordReader::operator & (TElement& _rElement)
    {
        return Read(_rElement);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline unsigned int CRecordReader::BeginCollection()
    {
        unsigned int NumberOfElements;

        ReadBinary(&NumberOfElements, sizeof(NumberOfElements));

        return NumberOfElements;
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordReader::ReadCollection(TElement* _pElements, unsigned int _NumberOfElements)
    {
        bool IsPrimitive = SIsPrimitive<TElement>::Value;

        if (IsPrimitive)
        {
            ReadBinary(_pElements, _NumberOfElements * sizeof(*_pElements));
        }
        else
        {
            for (unsigned int IndexOfElement = 0; IndexOfElement < _NumberOfElements; ++IndexOfElement)
            {
                Read(_pElements[IndexOfElement]);
            }
        }
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordReader::EndCollection()
    {
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordReader::ReadPrimitive(TElement& _rElement)
    {
        ReadBinary(&_rElement, sizeof(_rElement));
    }

    // -----------------------------------------------------------------------------

    inline void CRecordReader::ReadBinary(void* _pBytes, unsigned int _NumberOfBytes)
    {
        // -----------------------------------------------------------------------------
        // Read current time
        // -----------------------------------------------------------------------------
        InternReadBinary(&m_Timecode, sizeof(m_Timecode));
        
        // -----------------------------------------------------------------------------
        // Read data
        // -----------------------------------------------------------------------------
        InternReadBinary(_pBytes, _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CRecordReader::ReadClass(TElement& _rElement)
    {
        SER::Private::CAccess::Read(*this, const_cast<TElement&>(_rElement));
    }

    // -----------------------------------------------------------------------------
 
    inline CRecordReader::CStream* CRecordReader::GetStream()
    {
        return m_pStream;
    }

    // -----------------------------------------------------------------------------

    inline double CRecordReader::PeekTimecode()
    {
        double Timecode;

        InternReadBinary(&Timecode, sizeof(Timecode));

        m_pStream->seekg(-sizeof(Timecode), m_pStream->cur);

        return Timecode;
    }

    // -----------------------------------------------------------------------------

    inline void CRecordReader::SkipTime()
    {
        m_Timer.SetTime(PeekTimecode());
    }

    // -----------------------------------------------------------------------------

    inline bool CRecordReader::IsEnd() const
    {
        return m_pStream->peek() == EOF;
    }

    // -----------------------------------------------------------------------------

    inline void CRecordReader::InternReadBinary(void* _pBytes, unsigned int _NumberOfBytes)
    {
        m_pStream->read(static_cast<char*>(_pBytes), _NumberOfBytes);
    }
} // namespace SER