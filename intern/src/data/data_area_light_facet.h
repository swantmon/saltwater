
#pragma once

#include "base/base_typedef.h"
#include "base/base_vector3.h"

namespace Dt
{
    class CAreaLightFacet
    {
    public:

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

        void SetRotation(float _Angle);
        float GetRotation() const;

        void SetWidth(float _Width);
        float GetWidth() const;

        void SetHeight(float _Height);
        float GetHeight() const;

        void SetIsTwoSided(bool _Flag);
        bool GetIsTwoSided() const;

        Base::Float3& GetLightness();
        const Base::Float3& GetLightness() const;

    public:

        void UpdateLightness();

    public:

        CAreaLightFacet();
        ~CAreaLightFacet();

    private:

        Base::Float3   m_Color;
        Base::Float3   m_Direction;
        Base::Float3   m_Lightness;
        float          m_Temperature;
        float          m_Intensity;
        float          m_Rotation;
        float          m_Width;
        float          m_Height;
        bool           m_IsTwoSided;
        bool           m_HasTemperature;
    };
} // namespace Dt