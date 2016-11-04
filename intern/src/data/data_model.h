//
//  data_model.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb3.h"
#include "base/base_string.h"
#include "base/base_typedef.h"

namespace Dt
{
    class CModel
    {       
    public:
        
        Base::CharString m_Modelfile;
        
        Base::AABB3Float m_AABB;

        unsigned int     m_GenFlag;
    };
} // namespace Dt
