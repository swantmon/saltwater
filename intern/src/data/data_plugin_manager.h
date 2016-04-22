//
//  data_plugin_manager.h
//  data
//
//  Created by Tobias Schwandt on 22/04/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_plugin_facet.h"

namespace Dt
{
namespace PluginManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CARControllerPluginFacet* CreateARControllerPlugin();
    CARTrackedObjectPluginFacet* CreateARTrackedObjectPlugin();

    void Update();
} // namespace PluginManager
} // namespace Dt
