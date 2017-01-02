//
//  data_material_manager.h
//  data
//
//  Created by Tobias Schwandt on 22/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_material.h"

#include <functional>

namespace Dt
{
namespace MaterialManager
{
    typedef std::function<void(Dt::CMaterial* _pMaterial)> CMaterialDelegate;
} // namespace MaterialManager
} // namespace Dt

#define DATA_DIRTY_MATERIAL_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)

namespace Dt
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
        float             m_Displacement;
        Base::Float3      m_AlbedoColor;
        Base::Float4      m_TilingOffset;
        const Base::Char* m_pFileName;
	};
} // namespace Dt

namespace Dt
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    CMaterial& CreateMaterial(const SMaterialDescriptor& _rDescriptor);

    CMaterial& GetDefaultMaterial();

    CMaterial& GetMaterialByHash(unsigned int _Hash);

    void MarkMaterialAsDirty(CMaterial& _rMaterial, unsigned int _DirtyFlags);

    void RegisterDirtyMaterialHandler(CMaterialDelegate _NewDelegate);
} // namespace MaterialManager
} // namespace Dt