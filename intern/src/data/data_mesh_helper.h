
#pragma once

#include "data/data_mesh_component.h"

#include <string>

namespace Dt
{
namespace MeshHelper
{
    
} // namespace MeshHelper
} // namespace Dt

namespace Dt
{
namespace MeshHelper
{
    CMeshComponent* CreateMeshFromFile(const std::string& _rFileName, int _GenFlag);

    CMeshComponent* CreateMeshFromAssimp(const std::string& _pFileName, int _GenFlag, int _MeshIndex, const void* _pImporter);
} // namespace MeshManager
} // namespace Dt