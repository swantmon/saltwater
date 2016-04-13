
//
//  mr_marker.h
//  mr
//
//  Created by Tobias Schwandt on 07/10/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_managed_pool.h"

namespace MR
{
    class CMarker : public Base::CManagedPoolItemBase
    {
    public:

        void*        m_pHandle;
        unsigned int m_ID;
        unsigned int m_UserID;
        float        m_WidthInMeter;
        unsigned int m_AppearCounter;
        bool         m_IsRegistered;
        bool         m_IsFound;
    };
} // namespace MR

namespace MR
{
    typedef Base::CManagedPoolItemPtr<CMarker> CMarkerPtr;
} // namespace MR