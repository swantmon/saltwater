
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"
#include "base/base_serialize_glm.h"
#include "base/base_serialize_std_string.h"
#include "base/base_typedef.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CAreaLightComponent : public CComponent<CAreaLightComponent>
    {
    public:

        void SetColor(const glm::vec3& _rColor);
        glm::vec3& GetColor();
        const glm::vec3& GetColor() const;

        void SetTexture(const std::string& _pTexture2D);
        const std::string& GetTexture();

        bool GetHasTexture() const;

        void SetDirection(const glm::vec3& _rDirection);
        glm::vec3& GetDirection();
        const glm::vec3& GetDirection() const;

        void SetTemperature(float _Temperature);
        float GetTemperature() const;

        void EnableTemperature(bool _Flag);
        bool HasTemperature() const;

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

        void SetRotation(float _Angle);
        float GetRotation() const;

        void SetWidth(float _Width);
        float GetWidth() const;

        void SetHeight(float _Height);
        float GetHeight() const;

        void SetIsTwoSided(bool _Flag);
        bool GetIsTwoSided() const;

        glm::vec3& GetLightness();
        const glm::vec3& GetLightness() const;

    public:

        void UpdateLightness();

    public:

        CAreaLightComponent();
        ~CAreaLightComponent();

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            Base::Serialize(_rCodec, m_Color);
            Base::Serialize(_rCodec, m_Direction);
            Base::Serialize(_rCodec, m_Lightness);

            _rCodec >> m_Temperature;
            _rCodec >> m_Intensity;
            _rCodec >> m_Rotation;
            _rCodec >> m_Width;
            _rCodec >> m_Height;
            _rCodec >> m_IsTwoSided;
            _rCodec >> m_HasTemperature;

            Base::Serialize(_rCodec, m_Texture);
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            Base::Serialize(_rCodec, m_Color);
            Base::Serialize(_rCodec, m_Direction);
            Base::Serialize(_rCodec, m_Lightness);

            _rCodec << m_Temperature;
            _rCodec << m_Intensity;
            _rCodec << m_Rotation;
            _rCodec << m_Width;
            _rCodec << m_Height;
            _rCodec << m_IsTwoSided;
            _rCodec << m_HasTemperature;

            Base::Serialize(_rCodec, m_Texture);
        }

        inline IComponent* Allocate() override
        {
            return new CAreaLightComponent();
        }

    private:

        glm::vec3   m_Color;
        glm::vec3   m_Direction;
        glm::vec3   m_Lightness;
        float       m_Temperature;
        float       m_Intensity;
        float       m_Rotation;
        float       m_Width;
        float       m_Height;
        bool        m_IsTwoSided;
        bool        m_HasTemperature;
        std::string m_Texture;

    private:

        friend class CAreaLightComponentGUI;
    };
} // namespace Dt