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

#include "graphic/gfx_mesh.h"

namespace Gfx
{
    struct SMeshDescriptor
    {
        Dt::CMesh* m_pMesh;
    };
} // namespace Gfx

namespace Gfx
{
namespace MeshManager
{
    void OnStart();
    void OnExit();

    void Clear();
    
    CMeshPtr CreateMesh(const SMeshDescriptor& _rDescriptor);
    CMeshPtr CreateBox(float _Width, float _Height, float _Depth);
    CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
    CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement);
    CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices);
    CMeshPtr CreateRectangle(float _X, float _Y, float _Width, float _Height);
} // namespace MeshManager
} // namespace Gfx