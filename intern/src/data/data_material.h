
#pragma once

#include "base/base_include_glm.h"
#include "base/base_typedef.h"

#include "data/data_texture_2d.h"

#include <string>

namespace Dt
{
    class CMaterial
    {
    public:

        CMaterial();
        ~CMaterial();

        const std::string& GetMaterialname() const;

        const std::string& GetFileName() const;

        Dt::CTexture2D* GetColorTexture();
        const Dt::CTexture2D* GetColorTexture() const;

        Dt::CTexture2D* GetNormalTexture();
        const Dt::CTexture2D* GetNormalTexture() const;

        Dt::CTexture2D* GetRoughnessTexture();
        const Dt::CTexture2D* GetRoughnessTexture() const;

        Dt::CTexture2D* GetMetalTexture();
        const Dt::CTexture2D* GetMetalTexture() const;

        Dt::CTexture2D* GetAmbientOcclusionTexture();
        const Dt::CTexture2D* GetAmbientOcclusionTexture() const;

        Dt::CTexture2D* GetBumpTexture();
        const Dt::CTexture2D* GetBumpTexture() const;

        const glm::vec3& GetColor() const;

        const glm::vec4& GetTilingOffset() const;

        float GetRoughness() const;

        float GetSmoothness() const;

        float GetReflectance() const;

        float GetMetalness() const;

        float GetDisplacement() const;

        unsigned int GetHash() const;

    protected:
        
        std::string     m_Materialname;
        std::string     m_FileName;
        Dt::CTexture2D* m_pColorTexture;
        Dt::CTexture2D* m_pNormalTexture;
        Dt::CTexture2D* m_pRoughnessTexture;
        Dt::CTexture2D* m_pMetalTexture;
        Dt::CTexture2D* m_pAOTexture;
        Dt::CTexture2D* m_pBumpTexture;
        glm::vec3       m_Color;
        glm::vec4       m_TilingOffset;
        float           m_Roughness;
        float           m_Reflectance;
        float           m_MetalMask;
        float           m_Displacement;
        unsigned int    m_Hash;
    };
} // namespace Dt
