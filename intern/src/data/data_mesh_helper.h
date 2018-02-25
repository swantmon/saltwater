
#pragma once

#include "data/data_mesh_component.h"

namespace Dt
{
namespace MeshHelper
{
    CMeshComponent* CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag);
} // namespace MeshManager
} // namespace Dt