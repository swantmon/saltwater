//
//  data_fx_facet.h
//  data
//
//  Created by Tobias Schwandt on 29/03/16.
//  Copyright © 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

namespace Dt
{
    struct SFXType
    {
        enum Enum
        {
            Bloom,
            SSR,
            DOF,
            FXAA,
            SSAO,
            VolumeFog,
            UndefinedType = -1,
        };
    };
} // namespace Dt

namespace Dt
{
    class CBloomFXFacet
    {
    public:

        void SetTint(Base::Float4& _rTint);
        Base::Float4& GetTint();

        void SetIntensity(float _Intensity);
        float GetIntensity();

        void SetTreshhold(float _Treshhold);
        float GetTreshhold();

        void SetExposureScale(float _ExposureScale);
        float GetExposureScale();

        void SetSize(unsigned int _Size);
        unsigned int GetSize();

        void UpdateEffect();

    public:

        CBloomFXFacet();
        ~CBloomFXFacet();

    private:

        Base::Float4 m_Tint;                //> Modifies the brightness and color of each bloom. (Default: 1.0, 1.0, 1.0)
        float        m_Intensity;           //> Scales the color of the whole bloom effect. (Range: 0.0 - 8.0; Default: 1)
        float        m_Treshhold;           //> Defines how many luminance units a color needs to have to affect bloom. (Range: -1.0 - 8.0; Default: 1.0f)
        float        m_ExposureScale;       //> Defines a multiplier applied on bloom effect (Default: 2.0f)
        unsigned int m_Size;                //> Size of the bloom lights (Range: 1, 2, 3, 4, 5; Default: 3)
    };
} // namespace Dt

namespace Dt
{
    class CSSRFXFacet
    {
    public:

        void SetIntensity(float _Intensity);
        float GetIntensity();

        void SetRoughnessMask(float _RoughnessMask);
        float GetRoughnessMask();

        void SetUseLastFrame(bool _Flag);
        bool GetUseLastFrame();

        void UpdateEffect();

    public:

        CSSRFXFacet();
        ~CSSRFXFacet();

    private:

        float m_Intensity;              //> Intensity of SSR on final image (Default: 1.0f)
        float m_RoughnessMask;          //> Identifier of the roughness depending surfaces. (Default: -6.66f)
        bool  m_UseLastFrame;           //> Either use double reflections of not (Default: true)
    };
} // namespace Dt

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

namespace Dt
{
    class CFXAAFXFacet
    {
    public:

        void SetLuma(Base::Float3& _rLuma);
        Base::Float3& GetLuma();

        void UpdateEffect();

    public:

        CFXAAFXFacet();
        ~CFXAAFXFacet();

    private:

        Base::Float3 m_Luma;            //< Tone for AA (Default: 0.299f, 0.587f, 0.144f)
    };
} // namespace Dt

namespace Dt
{
    class CSSAOFXFacet
    {
    public:

        CSSAOFXFacet();
        ~CSSAOFXFacet();
    };
} // namespace Dt

namespace Dt
{
    class CVolumeFogFXFacet
    {
    public:

        void SetWindDirection(const Base::Float4& _rWindDirection);
        Base::Float4& GetWindDirection();

        void SetFogColor(const Base::Float4& _rFogColor);
        Base::Float4& GetFogColor();

        void SetFrustumDepthInMeter(float _FrustumDepthInMeter);
        float GetFrustumDepthInMeter();

        void SetShadowIntensity(float _ShadowIntensity);
        float GetShadowIntensity();

        void SetVolumetricFogScatteringCoefficient(float _VolumetricFogScatteringCoefficient);
        float GetVolumetricFogScatteringCoefficient();

        void SetVolumetricFogAbsorptionCoefficient(float _VolumetricFogAbsorptionCoefficient);
        float GetVolumetricFogAbsorptionCoefficient();

    public:

        CVolumeFogFXFacet();
        ~CVolumeFogFXFacet();

    private:

        Base::Float4 m_WindDirection;                      //< Wind direction of the fog (xyz = direction, w = speed)
        Base::Float4 m_FogColor;                           //< Overall color of the fog (rgb = color, a = intensity)
        float        m_FrustumDepthInMeter;                //< Meters of the fog will be calculated
        float        m_ShadowIntensity;                    //< Intensity of the fog (higher is harder edges)
        float        m_VolumetricFogScatteringCoefficient; //< Scattering amount of light by the fog
        float        m_VolumetricFogAbsorptionCoefficient; //< Absorption amount of light by the fog
    };
} // namespace Dt