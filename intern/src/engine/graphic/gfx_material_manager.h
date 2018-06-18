
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

#include "engine/data/data_material_component.h"

#include "engine/graphic/gfx_material.h"

namespace Gfx
{
namespace MaterialManager
{
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();

    ENGINE_API CMaterialPtr CreateMaterialFromName(const std::string& _rMaterialname, Dt::CMaterialComponent* _pComponent = nullptr);

    ENGINE_API CMaterialPtr CreateMaterialFromXML(const std::string& _rPathToFile, Dt::CMaterialComponent* _pComponent = nullptr);

    ENGINE_API CMaterialPtr CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex, Dt::CMaterialComponent* _pComponent = nullptr);

    ENGINE_API CMaterialPtr GetMaterialByHash(const Gfx::CMaterial::BHash _Hash);

    ENGINE_API const CMaterialPtr GetDefaultMaterial();
} // namespace MaterialManager
} // namespace Gfx