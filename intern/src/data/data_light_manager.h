//
//  data_light_manager.h
//  data
//
//  Created by Tobias Schwandt on 24/03/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_light_facet.h"

namespace Dt
{
namespace LightManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CPointLightFacet* CreatePointLight();
    CSunLightFacet* CreateSunLight();
    CGlobalProbeLightFacet* CreateGlobalProbeLight();
    CSkyFacet* CreateSky();

    void Update();
} // namespace LightManager
} // namespace Dt
