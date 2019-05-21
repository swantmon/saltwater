
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"
#include "base/base_typedef.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CSunComponent : public CComponent<CSunComponent>
    {
    public:

        enum ERefreshMode
        {
            Static,         //< Light will be updated at any time the settings has changed
            Dynamic,        //< Light will be updated at every frame
        };

    public:

        void SetRefreshMode(ERefreshMode _RefreshMode);
        ERefreshMode GetRefreshMode();

        void SetColor(const glm::vec3& _rColor);
        glm::vec3& GetColor();
        const glm::vec3& GetColor() const;

        void SetDirection(const glm::vec3& _rDirection);
        glm::vec3& GetDirection();
        const glm::vec3& GetDirection() const;

        void SetTemperature(float _Temperature);
        float GetTemperature() const;

        void EnableTemperature(bool _Flag);
        bool HasTemperature() const;

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

        float GetSunAngularRadius() const;

        glm::vec3& GetLightness();
        const glm::vec3& GetLightness() const;

    public:

        void UpdateLightness();

    public:

        CSunComponent();
        ~CSunComponent();

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            int RefreshMode;

            _rCodec >> RefreshMode;
            _rCodec >> m_Direction[0];
            _rCodec >> m_Direction[1];
            _rCodec >> m_Direction[2];
            _rCodec >> m_Color[0];
            _rCodec >> m_Color[1];
            _rCodec >> m_Color[2];
            _rCodec >> m_Lightness[0];
            _rCodec >> m_Lightness[1];
            _rCodec >> m_Lightness[2];
            _rCodec >> m_Temperature;
            _rCodec >> m_Intensity;
            _rCodec >> m_HasTemperature;

            m_RefreshMode = (ERefreshMode)RefreshMode;
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            _rCodec << (int)m_RefreshMode;
            _rCodec << m_Direction[0];
            _rCodec << m_Direction[1];
            _rCodec << m_Direction[2];
            _rCodec << m_Color[0];
            _rCodec << m_Color[1];
            _rCodec << m_Color[2];
            _rCodec << m_Lightness[0];
            _rCodec << m_Lightness[1];
            _rCodec << m_Lightness[2];
            _rCodec << m_Temperature;
            _rCodec << m_Intensity;
            _rCodec << m_HasTemperature;
        }

        inline IComponent* Allocate() override
        {
            return new CSunComponent();
        }

    private:

        ERefreshMode m_RefreshMode;
        glm::vec3    m_Direction;
        glm::vec3    m_Color;
        glm::vec3    m_Lightness;
        float        m_Temperature;
        float        m_Intensity;
        bool         m_HasTemperature;

    private:

        friend class CSunComponentGUI;
    };
} // namespace Dt