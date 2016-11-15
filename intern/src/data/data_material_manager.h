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
	struct SMaterialFileDescriptor 
	{
        const Base::Char* m_pFileName;
	};
} // namespace Dt

namespace Dt
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    // TODO by tschwandt
    // Remove creation of empty material
    CMaterial& CreateEmptyMaterial();


    CMaterial& CreateMaterial(const SMaterialFileDescriptor& _rDescriptor);

    void MarkMaterialAsDirty(CMaterial& _rMaterial, unsigned int _DirtyFlags);

    void RegisterDirtyMaterialHandler(CMaterialDelegate _NewDelegate);
} // namespace MaterialManager
} // namespace Dt