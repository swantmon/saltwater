//
//  gfx_model_manager.h
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"

#include "data/data_model.h"

#include "graphic/gfx_model.h"

namespace Gfx
{
    struct SModelDescriptor
    {
        Dt::CModel* m_pModel;
    };
} // namespace Gfx

namespace Gfx
{
namespace ModelManager
{
    void OnStart();
    void OnExit();

    void Clear();
    
    CModelPtr CreateModel(const SModelDescriptor& _rDescriptor);
    
    CModelPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
    CModelPtr CreateRectangle(float _X, float _Y, float _Width, float _Height);
} // namespace ModelManager
} // namespace Gfx