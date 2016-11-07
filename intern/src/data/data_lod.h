//
//  data_lod.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_surface.h"

namespace Dt
{
    class CLOD
    {
    public:
        
        static const unsigned int s_NumberOfSurfaces = 16;
        
    public:
        
        unsigned int m_NumberOfSurfaces;
        CSurface*    m_Surfaces[s_NumberOfSurfaces];
    };
} // namespace Dt
