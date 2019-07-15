
#pragma once

#include "base/base_defines.h"
#include "base/base_serialize_archive.h"

#include <assert.h>
#include <sstream>

namespace SER
{
    class CBinaryReader : public CArchive
    {
    public:
        enum
        {
            IsWriter = false,
            IsReader = true,
        };

    public:
        using CStream = std::istream;
        using CThis = CBinaryReader;

    public:
        inline  CBinaryReader(CStream& _rStream, unsigned int _Version);
        inline ~CBinaryReader();

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
        CStream* m_pStream;
    };
} // namespace SER

namespace SER
{
    inline CBinaryReader::CBinaryReader(CStream& _rStream, unsigned int _Version)
        : CArchive (_Version)
        , m_pStream(&_rStream)
    {
        // -----------------------------------------------------------------------------
        // Read header informations (internal format, version)
        // -----------------------------------------------------------------------------        
        ReadBinary(&m_ArchiveVersion, sizeof(m_ArchiveVersion));

        if (m_ArchiveVersion != _Version)
        {
            throw "Bad resource because of incompatible version.";
        }
    }

    // -----------------------------------------------------------------------------

    inline CBinaryReader::~CBinaryReader()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline CBinaryReader::CThis& CBinaryReader::Read(TElement& _rElement)
    {
        DispatchRead(*this, _rElement);

        return *this;
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CBinaryReader::CThis& CBinaryReader::operator >> (TElement& _rElement)
    {
        return Read(_rElement);
    }

    // -----------------------------------------------------------------------------


    template<typename TElement>
    inline CBinaryReader::CThis& CBinaryReader::operator & (TElement& _rElement)
    {
        return Read(_rElement);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline unsigned int CBinaryReader::BeginCollection()
    {
        int NumberOfElements;

        ReadBinary(&NumberOfElements, sizeof(NumberOfElements));

        return NumberOfElements;
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryReader::ReadCollection(TElement* _pElements, unsigned int _NumberOfElements)
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
    inline void CBinaryReader::EndCollection()
    {

    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryReader::ReadPrimitive(TElement& _rElement)
    {
        ReadBinary(&_rElement, sizeof(_rElement));
    }

    // -----------------------------------------------------------------------------

    inline void CBinaryReader::ReadBinary(void* _pBytes, unsigned int _NumberOfBytes)
    {
        m_pStream->read(static_cast<char*>(_pBytes), _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    template<typename TElement>
    inline void CBinaryReader::ReadClass(TElement& _rElement)
    {
        SER::Private::CAccess::Read(*this, const_cast<TElement&>(_rElement));
    }
} // namespace SER