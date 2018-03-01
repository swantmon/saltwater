//
//  gfx_lod.h
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_surface.h"

namespace Gfx
{
    class CLOD : public Base::CManagedPoolItemBase
    {
    public:
        
        CSurfacePtr GetSurface() const;
        
    protected:
        
        CLOD();
        ~CLOD();
        
    protected:
        
        CSurfacePtr m_Surface;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CLOD> CLODPtr;
} // namespace Gfx