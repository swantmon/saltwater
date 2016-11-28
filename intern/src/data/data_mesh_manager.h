
#pragma once

#include "data/data_mesh_facet.h"

namespace Dt
{
namespace MeshManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CMeshActorFacet* CreateModelActor();

    void Update();
} // namespace MeshManager
} // namespace Dt
