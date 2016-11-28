
#pragma once

#include "data/data_ar_actor_facet.h"

namespace Dt
{
namespace ARActorManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CARActorFacet* CreateARActor();

    void Update();
} // namespace ARMeshManager
} // namespace Dt
