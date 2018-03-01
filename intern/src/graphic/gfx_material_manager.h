
#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_material.h"

namespace Gfx
{
    struct SMaterialDescriptor
    {
        const char* m_pMaterialName;
        const char* m_pColorMap;
        const char* m_pNormalMap;
        const char* m_pRoughnessMap;
        const char* m_pMetalMaskMap;
        const char* m_pAOMap;
        const char* m_pBumpMap;
        float       m_Roughness;
        float       m_Reflectance;
        float       m_MetalMask;
        float       m_Displacement;
        glm::vec3   m_AlbedoColor;
        glm::vec4   m_TilingOffset;
    };

    struct SExternalImporter
    {
        enum EExternalImporter
        {
            Assimp,
        };
    };
} // namespace Gfx

namespace Gfx
{
namespace MaterialManager
{
    void OnStart();
    void OnExit();

    CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor);

    CMaterialPtr CreateMaterialFromFile(const Base::Char* _pFilename, int _MaterialIndex = 0);

    const CMaterialPtr GetDefaultMaterial();
} // namespace MaterialManager
} // namespace Gfx