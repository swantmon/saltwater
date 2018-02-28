
#pragma once

#include "data/data_mesh_component.h"

#include <string>

namespace Dt
{
namespace MeshHelper
{
    CMeshComponent* CreateMeshFromFile(const std::string& _rFileName, int _GenFlag, int _MeshIndex = 0);
} // namespace MeshManager
} // namespace Dt