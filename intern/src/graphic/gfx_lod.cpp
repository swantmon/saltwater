//
//  gfx_lod.cpp
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "gfx_lod.h"

namespace Gfx
{
    CLOD::CLOD()
        : m_NumberOfSurfaces(0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CLOD::~CLOD()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CLOD::GetNumberOfSurfaces() const
    {
        return m_NumberOfSurfaces;
    }
    
    // -----------------------------------------------------------------------------
    
    CSurfacePtr CLOD::GetSurface(unsigned int _Index) const
    {
        assert(_Index < s_NumberOfSurfaces);
        
        return m_Surfaces[_Index];
    }
} // namespace Gfx