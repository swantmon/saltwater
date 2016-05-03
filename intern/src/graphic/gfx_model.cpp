//
//  gfx_model.cpp
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "graphic/gfx_precompiled.h"

#include "gfx_model.h"

namespace Gfx
{
    CModel::CModel()
        : m_NumberOfLODs(0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CModel::GetNumberOfLODs() const
    {
        return m_NumberOfLODs;
    }
    
    // -----------------------------------------------------------------------------
    
    CLODPtr CModel::GetLOD(unsigned int _Index) const
    {
        assert(_Index < s_NumberOfLODs);
        
        return m_LODs[_Index];
    }
    
    // -----------------------------------------------------------------------------
    
    Base::AABB3Float CModel::GetAABB() const
    {
        return m_AABB;
    }
} // namespace Gfx