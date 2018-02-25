
#pragma once

#include "data/data_mesh.h"

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
namespace MeshManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CMesh& CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag);
    CMesh& CreatePredefinedMesh(CMesh::EPredefinedMesh _PredefinedMesh);

    void FreeMesh(CMesh& _rModel);
} // namespace MeshManager
} // namespace Dt