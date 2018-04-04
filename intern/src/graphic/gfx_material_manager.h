
#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_export.h"
#include "graphic/gfx_material.h"

namespace Gfx
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
} // namespace Gfx

namespace Gfx
{
namespace MaterialManager
{
    GFX_API void OnStart();
    GFX_API void OnExit();

    GFX_API CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor);

    GFX_API const CMaterialPtr GetDefaultMaterial();
} // namespace MaterialManager
} // namespace Gfx