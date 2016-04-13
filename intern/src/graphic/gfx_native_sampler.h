//
//  gfx_native_sampler.h
//  graphic
//
//  Created by Tobias Schwandt on 27/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_sampler.h"
#include "graphic/gfx_native_types.h"

namespace Gfx
{
    class CNativeSampler: public CSampler
    {
    public:
        
        Gfx::CNativeSamplerHandle m_NativeSampler;
    };
} // namespace Gfx