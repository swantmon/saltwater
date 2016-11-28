
#pragma once

#include "data/data_ar_tracked_object_facet.h"

namespace Dt
{
namespace ARTrackedObjectManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CARTrackedObjectPluginFacet* CreateARTrackedObjectPlugin();

    void Update();
} // namespace PluginManager
} // namespace Dt
