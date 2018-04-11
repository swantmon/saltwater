
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "engine/core/core_asset_generator.h"

#include "engine/graphic/gfx_mesh.h"

#include <string>

namespace Gfx
{
namespace MeshManager
{
    void OnStart();
    void OnExit();
    
    ENGINE_API CMeshPtr CreateMeshFromFile(const std::string& _rPathToFile, int _GenFlag, int _MeshIndex = 0);

    ENGINE_API CMeshPtr CreateBox(float _Width, float _Height, float _Depth);
    ENGINE_API CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
    ENGINE_API CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement);
    ENGINE_API CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices);
    ENGINE_API CMeshPtr CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height);
} // namespace MeshManager
} // namespace Gfx