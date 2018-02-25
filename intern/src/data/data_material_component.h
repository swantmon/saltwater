
#pragma once

#include "base/base_include_glm.h"

#include "data/data_component.h"

#include <string>

namespace Dt
{
    class CMaterialComponent : public CComponent<CMaterialComponent>
    {
    public:

        void SetMaterialname(const std::string& _rValue);
        const std::string& GetMaterialname() const;

        void SetFileName(const std::string& _rValue);
        const std::string& GetFileName() const;

        void SetColorTexture(const std::string& _rValue);
        const std::string& GetColorTexture() const;

        void SetNormalTexture(const std::string& _rValue);
        const std::string& GetNormalTexture() const;

        void SetRoughnessTexture(const std::string& _rValue);
        const std::string& GetRoughnessTexture() const;

        void SetMetalTexture(const std::string& _rValue);
        const std::string& GetMetalTexture() const;

        void SetAmbientOcclusionTexture(const std::string& _rValue);
        const std::string& GetAmbientOcclusionTexture() const;

        void SetBumpTexture(const std::string& _rValue);
        const std::string& GetBumpTexture() const;

        void SetColor(const glm::vec3& _rValue);
        const glm::vec3& GetColor() const;

        void SetTilingOffset(const glm::vec4& _rValue);
        const glm::vec4& GetTilingOffset() const;

        void SetRoughness(float _Value);
        float GetRoughness() const;

        void SetSmoothness(float _Value);
        float GetSmoothness() const;

        void SetReflectance(float _Value);
        float GetReflectance() const;

        void SetMetalness(float _Value);
        float GetMetalness() const;

        void SetDisplacement(float _Value);
        float GetDisplacement() const;

    public:

        CMaterialComponent();
        ~CMaterialComponent();

    private:

        std::string  m_Materialname;
        std::string  m_FileName;
        std::string  m_ColorTexture;
        std::string  m_NormalTexture;
        std::string  m_RoughnessTexture;
        std::string  m_MetalTexture;
        std::string  m_AOTexture;
        std::string  m_BumpTexture;
        glm::vec3    m_Color;
        glm::vec4    m_TilingOffset;
        float        m_Roughness;
        float        m_Reflectance;
        float        m_MetalMask;
        float        m_Displacement;
    };
} // namespace Dt