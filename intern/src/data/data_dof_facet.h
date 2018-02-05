
#pragma once

#include "base/base_lib_glm.h"

namespace Dt
{
    class CDOFFXFacet
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

        CDOFFXFacet();
        ~CDOFFXFacet();

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
    };
} // namespace Dt