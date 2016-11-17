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
        const char*       m_pMaterialName;
        const char*       m_pColorMap;
        const char*       m_pNormalMap;
        const char*       m_pRoughnessMap;
        const char*       m_pMetalMaskMap;
        const char*       m_pAOMap;
        const char*       m_pBumpMap;
        float             m_Roughness;
        float             m_Reflectance;
        float             m_MetalMask;
        Base::Float3      m_AlbedoColor;
        Base::Float4      m_TilingOffset;
        const Base::Char* m_pFileName;
    };
} // namespace Gfx

namespace Gfx
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor);

    CMaterialPtr GetDefaultMaterial();

    CMaterialPtr GetMaterialByHash(unsigned int _Hash);
} // namespace MaterialManager
} // namespace Gfx