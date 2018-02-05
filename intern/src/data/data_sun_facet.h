
#pragma once

#include "base/base_typedef.h"

#include "glm.hpp"

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

        glm::vec3& GetLightness();
        const glm::vec3& GetLightness() const;

    public:

        void UpdateLightness();

    public:

        CSunLightFacet();
        ~CSunLightFacet();

    private:

        ERefreshMode m_RefreshMode;
        glm::vec3 m_Direction;
        glm::vec3 m_Color;
        glm::vec3 m_Lightness;
        float        m_Temperature;
        float        m_Intensity;
        bool         m_HasTemperature;
    };
} // namespace Dt