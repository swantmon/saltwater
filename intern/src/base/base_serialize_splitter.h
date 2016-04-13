
////////////////////////////////////////////////////////////////////////////////
///
/// \file base_serialize_splitter.h
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
#include "base/base_logical_arithmetic.h"
#include "base/base_serialize_access.h"

namespace SER
{
    template<class TArchive, class TObject>
    struct SGlobalReader
    {
        inline static void Invoke(TArchive& _rArchive, TObject& _rObject)
        {
            Read(_rArchive, _rObject);
        }
    };

    template<class TArchive, class TObject>
    struct SGlobalWriter
    {
        inline static void Invoke(TArchive& _rArchive, const TObject& _rObject)
        {
            Write(_rArchive, _rObject);
        }
    };

    template<class TArchive, class TObject>
    struct SMemberReader
    {
        inline static void Invoke(TArchive& _rArchive, TObject& _rObject)
        {
            SER::Private::CAccess::Read(_rArchive, _rObject);
        }
    };

    template<class TArchive, class TObject>
    struct SMemberWriter
    {
        inline static void Invoke(TArchive& _rArchive, const TObject& _rObject)
        {
            SER::Private::CAccess::Write(_rArchive, _rObject);
        }
    };

    template<class TArchive, class TObject>
    inline void SplitGlobalSerialize(TArchive& _rArchive, TObject& _rObject)
    {
        typedef SGlobalWriter<TArchive, TObject> SWriter;
        typedef SGlobalReader<TArchive, TObject> SReader;

        typedef typename SIf<TArchive::IsWriter, SWriter, SReader>::X SActor;

        SActor::Invoke(_rArchive, _rObject);
    }

    template<class TArchive, class TObject>
    inline void SplitMemberSerialize(TArchive& _rArchive, TObject& _rObject)
    {
        typedef SMemberWriter<TArchive, TObject> SWriter;
        typedef SMemberReader<TArchive, TObject> SReader;

        typedef typename SIf<TArchive::IsWriter, SWriter, SReader>::X SActor;

        SActor::Invoke(_rArchive, _rObject);
    }
} // namespace SER


#define SPLIT_MEMBER_SERIALIZE()                                        \
    template<class TArchive>                                            \
    void Serialize(TArchive& _rArchive)                                 \
    {                                                                   \
        SER::SplitMemberSerialize(_rArchive, *this);              \
    }