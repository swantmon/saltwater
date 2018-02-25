
#pragma once

#include "base/base_include_glm.h"

#include <string>

namespace Dt
{
    class CMaterial;
} // namespace Dt

namespace Dt
{
	struct SMaterialDescriptor 
	{
        std::string m_pMaterialName;
        std::string m_pColorMap;
        std::string m_pNormalMap;
        std::string m_pRoughnessMap;
        std::string m_pMetalMaskMap;
        std::string m_pAOMap;
        std::string m_pBumpMap;
        float       m_Roughness;
        float       m_Reflectance;
        float       m_MetalMask;
        float       m_Displacement;
        glm::vec3   m_AlbedoColor;
        glm::vec4   m_TilingOffset;
        std::string m_pFileName;
	};
} // namespace Dt

namespace Dt
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    CMaterial& CreateMaterial(const SMaterialDescriptor& _rDescriptor);

    CMaterial& GetDefaultMaterial();

    CMaterial& GetMaterialByHash(unsigned int _Hash);
} // namespace MaterialManager
} // namespace Dt