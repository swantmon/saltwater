
#pragma once

#include "data/data_camera_actor_facet.h"

namespace Dt
{
namespace CameraActorManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CCameraActorFacet* CreateCameraActor();

    void Update();
} // namespace CameraActorManager
} // namespace Dt
