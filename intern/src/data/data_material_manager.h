
#pragma once

#include "base/base_typedef.h"

#include "data/data_material.h"

#include <string>

namespace Dt
{
namespace MaterialManager
{
    CMaterial* CreateMaterialFromName(const std::string& _rMaterialname);

    CMaterial* CreateMaterialFromXML(const std::string& _rPathToFile);

    CMaterial* CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex);

    CMaterial* GetMaterialByHash(const CMaterial::BHash _Hash);

    CMaterial* GetDefaultMaterial();
} // namespace MaterialManager
} // namespace Dt