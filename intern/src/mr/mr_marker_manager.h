
//
//  mr_marker_manager.h
//  mr
//
//  Created by Tobias Schwandt on 07/10/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "mr/mr_marker.h"

namespace MR
{
    struct SMarkerDescription
    {
        enum EMarkerType
        {
            Square,
            SquareBarcode,
            Multimarker,
            NFT,
            NumberOfMarkerTypes,
            UndefinedMarker = -1
        };

        unsigned int m_UserID;
        EMarkerType  m_Type;
        const char*  m_pPatternFile;
        float        m_WidthInMeter;
    };
} // namespace MR

namespace MR
{
namespace MarkerManager
{
    void OnStart();
    void OnExit();

    CMarkerPtr CreateMarker(const SMarkerDescription& _rDescription);
    
    CMarkerPtr GetMarkerByID(unsigned int _ID);

    void Clear();
} // namespace MarkerManager
} // namespace MR