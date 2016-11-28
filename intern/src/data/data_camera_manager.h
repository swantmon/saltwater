
#pragma once

#include "data/data_camera_facet.h"

namespace Dt
{
namespace CameraManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CCameraActorFacet* CreateCameraActor();

    void Update();
} // namespace CameraManager
} // namespace Dt
