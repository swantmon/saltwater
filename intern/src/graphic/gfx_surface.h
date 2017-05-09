//
//  gfx_surface.h
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_material.h"

namespace Gfx
{
    class CSurface : public Base::CManagedPoolItemBase
    {
    public:
        
#pragma warning(disable:4201)
        struct SSurfaceKey
        {
            typedef unsigned char BSurfaceID;
            
            union
            {
                struct
                {
                    BSurfaceID m_HasPosition           :  1;        //< Defines either a position is set
                    BSurfaceID m_HasNormal             :  1;        //< Defines either a normal is set
                    BSurfaceID m_HasTangent            :  1;        //< Defines either a tangent is set
                    BSurfaceID m_HasBitangent          :  1;        //< Defines either a bitangent is set
                    BSurfaceID m_HasTexCoords          :  1;        //< Defines either a texcoord is set
                    BSurfaceID m_Padding               :  3;
                };
                BSurfaceID m_Key;
            };            
        };
#pragma warning(default:4201)

    public:
        
        CBufferSetPtr GetVertexBuffer() const;
        CBufferPtr    GetIndexBuffer() const;
        CMaterialPtr  GetMaterial() const;
        CShaderPtr    GetShaderVS() const;
        CShaderPtr    GetMVPShaderVS() const;
        unsigned int  GetNumberOfIndices() const;
        unsigned int  GetNumberOfVertices() const;
        
        const SSurfaceKey& GetKey() const;
        
    protected:
        
        CSurface();
        ~CSurface();
        
    protected:
        
        CBufferSetPtr m_VertexBufferPtr;
        CBufferPtr    m_IndexBufferPtr;
        CMaterialPtr  m_MaterialPtr;

        CShaderPtr m_VertexShaderPtr;
        CShaderPtr m_MVPVertexShaderPtr;
        
        unsigned int m_NumberOfIndices;
        unsigned int m_NumberOfVertices;
        
        SSurfaceKey m_SurfaceKey;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSurface> CSurfacePtr;
} // namespace Gfx
