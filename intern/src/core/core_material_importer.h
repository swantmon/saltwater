
#pragma once

#include "base/base_include_glm.h"

#include <string>

namespace Core
{
namespace MaterialImporter
{
    struct SMaterialDescriptor
    {
        std::string m_MaterialName;
        std::string m_ColorTexture;
        std::string m_NormalTexture;
        std::string m_RoughnessTexture;
        std::string m_MetalTexture;
        std::string m_AmbientOcclusionTexture;
        std::string m_BumpTexture;
        float       m_Roughness;
        float       m_Reflectance;
        float       m_MetalMask;
        float       m_Displacement;
        glm::vec3   m_AlbedoColor;
        glm::vec4   m_TilingOffset;
    };
} // namespace MaterialImporter
} // namespace Core

namespace Core
{
namespace MaterialImporter
{
    SMaterialDescriptor CreateDescriptionFromXML(const std::string& _rPathToFile);

    SMaterialDescriptor CreateDescriptionFromAssimp(const std::string& _rPathToFile, int _MaterialIndex = 0);
} // namespace MaterialImporter
} // namespace Core
