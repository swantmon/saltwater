
#pragma once

#include "base/base_typedef.h"
#include "base/base_vector3.h"

namespace Dt
{
    class CSunLightFacet
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

        Base::Float3& GetLightness();
        const Base::Float3& GetLightness() const;

    public:

        void UpdateLightness();

    public:

        CSunLightFacet();
        ~CSunLightFacet();

    private:

        ERefreshMode m_RefreshMode;
        Base::Float3 m_Direction;
        Base::Float3 m_Color;
        Base::Float3 m_Lightness;
        float        m_Temperature;
        float        m_Intensity;
        bool         m_HasTemperature;
    };
} // namespace Dt