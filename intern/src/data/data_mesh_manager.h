
#pragma once

#include "data/data_mesh_actor_facet.h"

namespace Dt
{
namespace MeshActorManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CMeshActorFacet* CreateMeshActor();

    void Update();
} // namespace MeshManager
} // namespace Dt
