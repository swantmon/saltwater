
#pragma once

#include "data/data_material_component.h"

#include <string>

namespace Dt
{
namespace MaterialHelper
{
    CMaterialComponent* CreateMaterialFromFile(const std::string& _rFilename);

    const CMaterialComponent* GetDefaultMaterial();
} // namespace MaterialHelper
} // namespace Dt