
#pragma once

#include "base/base_include_glm.h"

#include "core/core_material_importer.h"

#include "graphic/gfx_material.h"

namespace Gfx
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    CMaterialPtr CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor& _rDescriptor);

    const CMaterialPtr GetDefaultMaterial();
} // namespace MaterialManager
} // namespace Gfx