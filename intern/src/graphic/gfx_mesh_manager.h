
#pragma once

#include "base/base_typedef.h"

#include "core/core_asset_generator.h"

#include "graphic/gfx_export.h"
#include "graphic/gfx_mesh.h"

#include <string>

namespace Gfx
{
namespace MeshManager
{
    GFX_API void OnStart();
    GFX_API void OnExit();
    
    GFX_API CMeshPtr CreateMeshFromFile(const std::string& _rPathToFile, int _GenFlag, int _MeshIndex = 0);

    GFX_API CMeshPtr CreateBox(float _Width, float _Height, float _Depth);
    GFX_API CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
    GFX_API CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement);
    GFX_API CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices);
    GFX_API CMeshPtr CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height);
} // namespace MeshManager
} // namespace Gfx