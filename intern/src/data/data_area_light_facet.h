
#pragma once

#include "base/base_lib_glm.h"
#include "base/base_typedef.h"

#include "data/data_texture_2d.h"

namespace Dt
{
    class CAreaLightFacet
    {
    public:

        void SetColor(const glm::vec3& _rColor);
        glm::vec3& GetColor();
        const glm::vec3& GetColor() const;

        void SetTexture(Dt::CTexture2D* _pTexture2D);
        Dt::CTexture2D* GetTexture();

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

        CAreaLightFacet();
        ~CAreaLightFacet();

    private:

        glm::vec3    m_Color;
        glm::vec3    m_Direction;
        glm::vec3    m_Lightness;
        float           m_Temperature;
        float           m_Intensity;
        float           m_Rotation;
        float           m_Width;
        float           m_Height;
        bool            m_IsTwoSided;
        bool            m_HasTemperature;
        Dt::CTexture2D* m_pTexture;
    };
} // namespace Dt