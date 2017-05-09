//
//  gfx_surface.cpp
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "graphic/gfx_precompiled.h"

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
        m_VertexBufferPtr    = 0;
        m_IndexBufferPtr     = 0;
        m_MaterialPtr        = 0;
        m_VertexShaderPtr    = 0;
        m_MVPVertexShaderPtr = 0;
    }
    
    CBufferSetPtr CSurface::GetVertexBuffer() const
    {
        return m_VertexBufferPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CSurface::GetIndexBuffer() const
    {
        return m_IndexBufferPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterialPtr CSurface::GetMaterial() const
    {
        return m_MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CSurface::GetShaderVS() const
    {
        return m_VertexShaderPtr;
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CSurface::GetMVPShaderVS() const
    {
        return m_MVPVertexShaderPtr;
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