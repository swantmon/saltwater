
////////////////////////////////////////////////////////////////////////////////
///
/// \file base_serialize_access.h
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

namespace SER
{
namespace Private
{
	class CAccess
    {
    public:
        template <class TArchive, class TObject>
        static void Read(TArchive& _rCodec, TObject& _rObject);

        template <class TArchive, class TObject>
        static void Write(TArchive& _rCodec, TObject& _rObject);

        template <class TArchive, class TObject>
        static void Serialize(TArchive& _rCodec, TObject& _rObject);
    };
} // namespace Private
} // namespace SER

namespace SER
{
namespace Private
{
    template <class TArchive, class TObject>
    inline void CAccess::Read(TArchive& _rCodec, TObject& _rObject)
    {
        _rObject.Read(_rCodec);
    }

    // -----------------------------------------------------------------------------

    template <class TArchive, class TObject>
    inline void CAccess::Write(TArchive& _rCodec, TObject& _rObject)
    {
        _rObject.Write(_rCodec);
    }

    // -----------------------------------------------------------------------------

	template <class TArchive, class TObject>
    inline void CAccess::Serialize(TArchive& _rCodec, TObject& _rObject)
    {
        _rObject.Serialize(_rCodec);
    }
} // namespace Private
} // namespace SER