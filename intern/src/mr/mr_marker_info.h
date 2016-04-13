
//
//  mr_marker_info.h
//  mr
//
//  Created by Tobias Schwandt on 07/10/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_matrix3x3.h"
#include "base/base_vector3.h"

namespace MR
{
    class CMarkerInfo
    {
    public:

        unsigned int   m_UserID;
        unsigned int   m_FrameCounter;
        Base::Float3x3 m_RotationToCamera;
        Base::Float3   m_TranslationToCamera;
    };
} // namespace MR