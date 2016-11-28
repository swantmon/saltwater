
#pragma once

#include "data/data_dof_facet.h"

namespace Dt
{
namespace DOFManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CDOFFXFacet* CreateDOFFX();

    void Update();
} // namespace DOFManager
} // namespace Dt
