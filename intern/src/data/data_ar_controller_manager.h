
#pragma once

#include "data/data_ar_controller_facet.h"

namespace Dt
{
namespace ARControllerManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CARControllerPluginFacet* CreateARControllerPlugin();

    void Update();
} // namespace ARControllerManager
} // namespace Dt
