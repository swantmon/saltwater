
//
//  mr_tracker_manager.h
//  mr
//
//  Created by Tobias Schwandt on 07/10/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"

#include "mr/mr_marker.h"
#include "mr/mr_marker_info.h"

namespace MR
{
namespace TrackerManager
{
    void OnStart();
    void OnExit();

    void Clear();

    void Update();

    void RegisterMarker(CMarkerPtr _MarkerPtr);
    
    unsigned int PollMarker(CMarkerInfo* _pMarkerInfo);    
} // namespace TrackerManager
} // namespace MR
