
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_mesh.h"

namespace Gfx
{
namespace MeshManager
{
    void OnStart();
    void OnExit();
    
    CMeshPtr CreateMeshFromFile(const Base::Char* _pFilename, int _GenFlag);

    CMeshPtr CreateBox(float _Width, float _Height, float _Depth);
    CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
    CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement);
    CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices);
    CMeshPtr CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height);
} // namespace MeshManager
} // namespace Gfx