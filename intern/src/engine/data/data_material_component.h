
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

#include "engine/data/data_component.h"

#include <string>

namespace Dt
{
    class CMaterial;
} // namespace Dt

namespace Dt
{
    class ENGINE_API CMaterialComponent : public CComponent<CMaterialComponent>
    {
    public:

        typedef Base::BHash BHash;

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

        void SetAlphaTexture(const std::string& _rValue);
        const std::string& GetAlphaTexture() const;

        void SetColor(const glm::vec4& _rValue);
        const glm::vec4& GetColor() const;

        void SetTiling(const glm::vec2& _rValue);
        const glm::vec2 GetTiling() const;

        void SetOffset(const glm::vec2& _rValue);
        const glm::vec2 GetOffset() const;

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

        void SetAlpha(float _Value);
        float GetAlpha() const;

        void SetRefractionIndex(float _Value);
        float GetRefractionIndex() const;

    public:

        CMaterialComponent();
        ~CMaterialComponent();

    private:

        std::string m_Materialname;
        std::string m_FileName;
        std::string m_ColorTexture;
        std::string m_NormalTexture;
        std::string m_RoughnessTexture;
        std::string m_MetalTexture;
        std::string m_AmbientOcclusionTexture;
        std::string m_BumpTexture;
        std::string m_AlphaTexture;
        glm::vec4   m_Color;
        glm::vec4   m_TilingOffset;
        float       m_Roughness;
        float       m_Reflectance;
        float       m_MetalMask;
        float       m_Displacement;
        float       m_RefractionIndex;

    private:

        friend class CMaterialComponentGUI;
    };
} // namespace Dt