//
//  gfx_model.h
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb3.h"
#include "base/base_managed_pool.h"

#include "graphic/gfx_lod.h"

namespace Gfx
{
    class CMesh : public Base::CManagedPoolItemBase
    {
    public:
        
        static const unsigned int s_NumberOfLODs = 4;
        
    public:
        
        unsigned int GetNumberOfLODs() const;
        
        CLODPtr GetLOD(unsigned int _Index) const;
        
        Base::AABB3Float GetAABB() const;

    public:

        CMesh();
        
    protected:
        
        unsigned int      m_NumberOfLODs;
        CLODPtr           m_LODs[s_NumberOfLODs];
        Base::AABB3Float  m_AABB;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CMesh> CMeshPtr;
} // namespace Gfx