//
//  data_actor_manager.h
//  data
//
//  Created by Tobias Schwandt on 24/03/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_actor_facet.h"

namespace Dt
{
namespace ActorManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CModelActorFacet* CreateModelActor();
    CARActorFacet* CreateARActor();
    CCameraActorFacet* CreateCameraActor();

    void Update();
} // namespace ActorManager
} // namespace Dt
