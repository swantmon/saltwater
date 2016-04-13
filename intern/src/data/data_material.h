//
//  data_material.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "data/data_texture_2d.h"

namespace Dt
{
    class CMaterial
    {
    public:
        
        const Base::Char* m_pMaterialname;
        Dt::CTexture2D*   m_pColorMap;
        Dt::CTexture2D*   m_pNormalMap;
        Dt::CTexture2D*   m_pRoughnessMap;
        Dt::CTexture2D*   m_pReflectanceMap;
        Dt::CTexture2D*   m_pMetalMaskMap;
        Dt::CTexture2D*   m_pAOMap;
        Dt::CTexture2D*   m_pBumpMap;
        Base::Float3      m_Color;
        Base::Float4      m_TilingOffset;
        float             m_Roughness;
        float             m_Reflectance;
        float             m_MetalMask;
    };
} // namespace Dt
