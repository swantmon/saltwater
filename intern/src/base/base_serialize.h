
////////////////////////////////////////////////////////////////////////////////
///
/// \file base_serialize.h
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

namespace SER
{
    template <class TArchive, class TObject>
    inline void Serialize(TArchive& _rCodec, TObject& _rObject)
    {
        SER::Private::CAccess::Serialize(_rCodec, _rObject);
    }
} // namespace SER