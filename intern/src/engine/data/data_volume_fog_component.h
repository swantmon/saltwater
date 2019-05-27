
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"
#include "base/base_serialize_glm.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CVolumeFogComponent : public CComponent<CVolumeFogComponent>
    {
    public:

        void SetWindDirection(const glm::vec4& _rWindDirection);
        glm::vec4& GetWindDirection();

        void SetFogColor(const glm::vec4& _rFogColor);
        glm::vec4& GetFogColor();

        void SetFrustumDepthInMeter(float _FrustumDepthInMeter);
        float GetFrustumDepthInMeter();

        void SetShadowIntensity(float _ShadowIntensity);
        float GetShadowIntensity();

        void SetScatteringCoefficient(float _ScatteringCoefficient);
        float GetScatteringCoefficient();

        void SetAbsorptionCoefficient(float _AbsorptionCoefficient);
        float GetAbsorptionCoefficient();

        void SetDensityLevel(float _DensityLevel);
        float GetDensityLevel();

        void SetDensityAttenuation(float _DensityAttenuation);
        float GetDensityAttenuation();

    public:

        CVolumeFogComponent();
        ~CVolumeFogComponent();

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            Base::Serialize(_rCodec, m_WindDirection);
            Base::Serialize(_rCodec, m_FogColor);

            _rCodec >> m_FrustumDepthInMeter;
            _rCodec >> m_ShadowIntensity;
            _rCodec >> m_ScatteringCoefficient;
            _rCodec >> m_AbsorptionCoefficient;
            _rCodec >> m_DensityLevel;
            _rCodec >> m_DensityAttenuation;
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            Base::Serialize(_rCodec, m_WindDirection);
            Base::Serialize(_rCodec, m_FogColor);

            _rCodec << m_FrustumDepthInMeter;
            _rCodec << m_ShadowIntensity;
            _rCodec << m_ScatteringCoefficient;
            _rCodec << m_AbsorptionCoefficient;
            _rCodec << m_DensityLevel;
            _rCodec << m_DensityAttenuation;
        }

        inline IComponent* Allocate() override
        {
            return new CVolumeFogComponent();
        }

    private:

        glm::vec4 m_WindDirection;                      //< Wind direction of the fog (xyz = direction, w = speed)
        glm::vec4 m_FogColor;                           //< Overall color of the fog (rgb = color, a = intensity)
        float     m_FrustumDepthInMeter;                //< Meters of the fog will be calculated
        float     m_ShadowIntensity;                    //< Intensity of the fog (higher is harder edges)
        float     m_ScatteringCoefficient;              //< Scattering amount of light by the fog
        float     m_AbsorptionCoefficient;              //< Absorption amount of light by the fog
        float     m_DensityLevel;                       //< Amount of density at the ground level / sea level
        float     m_DensityAttenuation;                 //< Attenuation factor for the density at higher level

    private:

        friend class CVolumeFogComponentGUI;
    };
} // namespace Dt