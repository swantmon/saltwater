//
//  data_model.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb3.h"
#include "base/base_string.h"
#include "base/base_typedef.h"

#include "data/data_lod.h"

namespace Dt
{
    class CModel
    {
    public:
        
        static const unsigned int s_NumberOfLODs = 4;
        
    public:
        
        Base::CharString m_Modelname;
        unsigned int     m_NumberOfLODs;
        CLOD*            m_LODs[s_NumberOfLODs];
        Base::AABB3Float m_AABB;
    };
} // namespace Dt
