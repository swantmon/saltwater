//
//  gfx_surface.cpp
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "gfx_surface.h"

namespace Gfx
{
    CSurface::CSurface()
        : m_NumberOfIndices (0)
        , m_NumberOfVertices(0)
    {
        m_SurfaceKey.m_Key = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    CSurface::~CSurface()
    {
        
    }
    
    CBufferSetPtr CSurface::GetVertexBuffer() const
    {
        return m_VertexBuffer;
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CSurface::GetIndexBuffer() const
    {
        return m_IndexBuffer;
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterialPtr CSurface::GetMaterial() const
    {
        return m_MaterialPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CSurface::GetNumberOfIndices() const
    {
        return m_NumberOfIndices;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CSurface::GetNumberOfVertices() const
    {
        return m_NumberOfVertices;
    }

    // -----------------------------------------------------------------------------
    
    const CSurface::SSurfaceKey& CSurface::GetKey() const
    {
        return m_SurfaceKey;
    }
} // namespace Gfx