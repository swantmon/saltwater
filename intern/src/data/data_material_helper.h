
#pragma once

#include "core/core_material_importer.h"

#include "data/data_material_component.h"

#include <string>

namespace Dt
{
namespace MaterialHelper
{
    CMaterialComponent* CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor _rDescriptor);

    const CMaterialComponent* GetDefaultMaterial();
} // namespace MaterialHelper
} // namespace Dt