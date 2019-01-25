
#pragma once

#include "base/base_defines.h"
#include "base/base_serialize_splitter.h"

#include <vector>

namespace SER
{
	template<class TArchive, typename TObject>
    inline void Write(TArchive& _rArchive, const std::vector<TObject>& _rVector);

    template<class TArchive, typename TObject>
    inline void Read(TArchive& _rArchive, std::vector<TObject>& _rVector);

    template<class TArchive, typename TObject>
    inline void Serialize(TArchive& _rArchive, std::vector<TObject>& _rVector);
} // namespace SER

namespace SER
{
	template<class TArchive, typename TObject>
    inline void Write(TArchive& _rArchive, const std::vector<TObject>& _rVector)
    {
        unsigned int NumberOfElements = static_cast<unsigned int>(_rVector.size());

        _rArchive.template BeginCollection<TObject>(NumberOfElements);
        
        if (NumberOfElements > 0)
        {
            _rArchive.WriteCollection(&_rVector.front(), NumberOfElements);
        }

        _rArchive.template EndCollection<TObject>();
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename TObject>
    inline void Read(TArchive& _rArchive, std::vector<TObject>& _rVector)
    {
        unsigned int NumberOfElements = 0;

        NumberOfElements = _rArchive.template BeginCollection<TObject>();

        if (NumberOfElements > 0)
        {
            _rVector.resize(NumberOfElements);

            _rArchive.ReadCollection(&_rVector.front(), NumberOfElements);
        }
        
        _rArchive.template EndCollection<TObject>();
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename TObject>
    inline void Serialize(TArchive& _rArchive, std::vector<TObject>& _rVector)
    {
        SER::SplitGlobalSerialize(_rArchive, _rVector);
    }
} // namespace SER