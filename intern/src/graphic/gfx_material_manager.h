//
//  gfx_material_manager.h
//  graphic
//
//  Created by Tobias Schwandt on 22/04/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "data/data_material.h"

#include "graphic/gfx_material.h"
#include "graphic/gfx_surface.h"

namespace Gfx
{
    struct SMaterialDescriptor
    {
        CSurface::SSurfaceKey::BSurfaceID m_ID;
        Dt::CMaterial*                    m_pMaterial;
    };
} // namespace Gfx

namespace Gfx
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor);
} // namespace MaterialManager
} // namespace Gfx