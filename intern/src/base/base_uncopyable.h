//
//  base_uncopyable.h
//  base
//
//  Created by Tobias Schwandt on 09/03/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"

namespace PAT
{
    class CUncopyable
    {
    protected:
        
        inline CUncopyable();
        inline ~CUncopyable();
        
    private:
        
        CUncopyable(const CUncopyable&);
        
        CUncopyable& operator = (const CUncopyable&);
    };
} // namespace PAT

namespace PAT
{
    inline CUncopyable::CUncopyable()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CUncopyable::~CUncopyable()
    {
    }
} // namespace PAT
