
#pragma once

#include "base/base_typedef.h"
#include "base/base_vector3.h"

namespace Dt
{
    class CPointLightFacet
    {
    public:

        enum EShadowType
        {
            NoShadows,
            HardShadows,
            GlobalIllumination,
            NumberOfTypes,
        };

        enum EShadowQuality
        {
            Low,
            Medium,
            High,
            VeryHigh,
            NumberOfQualities,
        };

        enum ERefreshMode
        {
            Static,         //< Light will be updated at any time the settings has changed
            Dynamic,        //< Light will be updated at every frame
        };

    public:

        void SetShadowType(EShadowType _ShadowType);
        EShadowType GetShadowType();

        void SetShadowQuality(EShadowQuality _ShadowQuality);
        EShadowQuality GetShadowQuality();

        void SetRefreshMode(ERefreshMode _RefreshMode);
        ERefreshMode GetRefreshMode();

        void SetColor(const Base::Float3& _rColor);
        Base::Float3& GetColor();
        const Base::Float3& GetColor() const;

        void SetDirection(const Base::Float3& _rDirection);
        Base::Float3& GetDirection();
        const Base::Float3& GetDirection() const;

        void SetTemperature(float _Temperature);
        float GetTemperature() const;

        void EnableTemperature(bool _Flag);
        bool HasTemperature() const;

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

        void SetAttenuationRadius(float _AttenuationRadius);
        float GetAttenuationRadius() const;
        float GetReciprocalSquaredAttenuationRadius() const;

        void SetInnerConeAngle(float _Angle);
        float GetInnerConeAngle() const;

        void SetOuterConeAngle(float _Angle);
        float GetOuterConeAngle() const;

        float GetAngleScale() const;
        float GetAngleOffset() const;

        Base::Float3& GetLightness();
        const Base::Float3& GetLightness() const;

    public:

        void UpdateLightness();

    public:

        CPointLightFacet();
        ~CPointLightFacet();

    private:
        EShadowType    m_ShadowType;
        EShadowQuality m_ShadowQuality;
        ERefreshMode   m_RefreshMode;
        Base::Float3   m_Color;
        Base::Float3   m_Direction;
        Base::Float3   m_Lightness;
        float          m_Temperature;
        float          m_Intensity;
        float          m_AttentuationRadius;
        float          m_ReciprocalSquaredAttentuationRadius;
        float          m_InnerConeAngle;
        float          m_OuterConeAngle;
        float          m_AngleScale;
        float          m_AngleOffset;
        bool           m_HasTemperature;
    };
} // namespace Dt