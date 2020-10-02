
#pragma once

#include "base/base_defines.h"
#include "base/base_serialize_splitter.h"

#include <string>

namespace SER
{
    template<class TArchive, typename TObject, class TTraits, class TAllocator>
    inline void Write(TArchive& _rArchive, const std::basic_string<TObject, TTraits, TAllocator>& _rString);

    template<class TArchive, typename TObject, class TTraits, class TAllocator>
    inline void Read(TArchive& _rArchive, std::basic_string<TObject, TTraits, TAllocator>& _rString);

    template<class TArchive, typename TObject, class TTraits, class TAllocator>
    inline void Serialize(TArchive& _rArchive, std::basic_string<TObject, TTraits, TAllocator>& _rString);
} // namespace SER

namespace SER
{
    template<class TArchive, typename TObject, class TTraits, class TAllocator>
    inline void Write(TArchive& _rArchive, const std::basic_string<TObject, TTraits, TAllocator>& _rString)
    {
        _rArchive.template BeginCollection<TObject>(static_cast<unsigned int>(_rString.size()));
        _rArchive.WriteCollection(_rString.data(), static_cast<unsigned int>(_rString.size()));
        _rArchive.template EndCollection<TObject>();
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename TObject, class TTraits, class TAllocator>
    inline void Read(TArchive& _rArchive, std::basic_string<TObject, TTraits, TAllocator>& _rString)
    {
        unsigned int NumberOfElements;

        NumberOfElements = _rArchive.template BeginCollection<TObject>();

        _rString.clear();
        _rString.resize(NumberOfElements);

        _rArchive.ReadCollection(const_cast<char*>(_rString.data()), NumberOfElements);
        _rArchive.template EndCollection<TObject>();
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename TObject, class TTraits, class TAllocator>
    inline void Serialize(TArchive& _rArchive, std::basic_string<TObject, TTraits, TAllocator>& _rString)
    {
        SER::SplitGlobalSerialize(_rArchive, _rString);
    }
} // namespace SER