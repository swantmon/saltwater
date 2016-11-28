
#pragma once

#include "base/base_vector2.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

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

        Base::Float2& GetNear();
        Base::Float4& GetLerpScale();
        Base::Float4& GetLerpBias();
        Base::Float3& GetEqFar();

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

        Base::Float2 m_Near;             //< Distance up to everything should be blured (y = 1; x = -y / Far_Distance)
        Base::Float4 m_LerpScale;
        Base::Float4 m_LerpBias;
        Base::Float3 m_EqFar;            //< Distance since everything should be blured (x = 1 / 1 - Start_Distance; y = 1 - x)
    };
} // namespace Dt