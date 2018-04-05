
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "data/data_material.h"

#include <string>

namespace Dt
{
namespace MaterialManager
{
    ENGINE_API CMaterial* CreateMaterialFromName(const std::string& _rMaterialname);

    ENGINE_API CMaterial* CreateMaterialFromXML(const std::string& _rPathToFile);

    ENGINE_API CMaterial* CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex);

    ENGINE_API CMaterial* GetMaterialByHash(const CMaterial::BHash _Hash);

    ENGINE_API CMaterial* GetDefaultMaterial();
} // namespace MaterialManager
} // namespace Dt