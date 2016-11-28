
#pragma once

namespace Dt
{
    class CSSRFXFacet
    {
    public:

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

        void SetRoughnessMask(float _RoughnessMask);
        float GetRoughnessMask() const;

        void SetDistance(float _Distance);
        float GetDistance() const;

        void SetUseLastFrame(bool _Flag);
        bool GetUseLastFrame() const;

        void UpdateEffect();

    public:

        CSSRFXFacet();
        ~CSSRFXFacet();

    private:

        float m_Intensity;              //> Intensity of SSR on final image (Default: 1.0f)
        float m_RoughnessMask;          //> Identifier of the roughness depending surfaces. (Default: -6.66f)
        float m_Distance;               //> Distance in units between objects to detect reflection (equal to ray length)
        bool  m_UseLastFrame;           //> Either use double reflections of not (Default: true)
    };
} // namespace Dt