//
//  data_material_manager.h
//  data
//
//  Created by Tobias Schwandt on 22/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_material.h"

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

    void Clear();

    CMaterial& CreateEmptyMaterial();
    CMaterial& CreateMaterial(const SMaterialFileDescriptor& _rDescriptor);
    
    void FreeMaterial(CMaterial& _rMaterial);
} // namespace MaterialManager
} // namespace Dt