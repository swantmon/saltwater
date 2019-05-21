
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CDOFComponent : public CComponent<CDOFComponent>
    {
    public:

        void SetNearDistance(float _NearDistance);
        float GetNearDistance();

        void SetFarDistance(float _FarDistance);
        float GetFarDistance();

        void SetNearToFarRatio(float _NearToFarRatio);
        float GetNearToFarRatio();
        
        void SetFadeUnToSmallBlur(float _FadeUnToSmallBlur);
        float GetFadeUnToSmallBlur();

        void SetFadeSmallToMediumBlur(float _FadeSmallToMediumBlur);
        float GetFadeSmallToMediumBlur();

        glm::vec2& GetNear();
        glm::vec4& GetLerpScale();
        glm::vec4& GetLerpBias();
        glm::vec3& GetEqFar();

        void UpdateEffect();

    public:

        CDOFComponent();
        ~CDOFComponent();

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CDOFComponent();
        }

    private:

        float m_NearDistance;            //< End of near blur (Default: 0.01f)
        float m_FarDistance;             //< Start of far blur (Default: 0.5f)
        float m_NearToFarRatio;          //< Distance between near and far blur (Default: 0.8f)
        float m_FadeUnToSmallBlur;       //< Fading between no blur to small blur (Default: 0.05f)
        float m_FadeSmallToMediumBlur;   //< Fading between small blur to medium blur (Default: 0.3f)

        glm::vec2 m_Near;             //< Distance up to everything should be blured (y = 1; x = -y / Far_Distance)
        glm::vec4 m_LerpScale;
        glm::vec4 m_LerpBias;
        glm::vec3 m_EqFar;            //< Distance since everything should be blured (x = 1 / 1 - Start_Distance; y = 1 - x)

    private:

        friend class CDOFComponentGUI;
    };
} // namespace Dt