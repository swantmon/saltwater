
#pragma once

#include "data/data_ar_mesh_facet.h"

namespace Dt
{
namespace ARMeshManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CARActorFacet* CreateARActor();

    void Update();
} // namespace ARMeshManager
} // namespace Dt
