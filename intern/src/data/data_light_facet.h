//
//  data_light_facet.h
//  data
//
//  Created by Tobias Schwandt on 23/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "data/data_texture_2d.h"
#include "data/data_texture_cube.h"

namespace Dt
{
    struct SLightType
    {
        enum Enum
        {
            Point,
            Sun,
            LightProbe,
            Sky,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt

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

namespace Dt
{
    // -----------------------------------------------------------------------------
    // Add generic light probe
    // -----------------------------------------------------------------------------
    class CLightProbeFacet
    {
    public:

        static const unsigned int s_NumberOfQualities = 5;

    public:

        enum EType
        {
            Sky,         //< Defines a global light probe that use the skydome
            Custom,      //< A cubemap texture can be set for generating the global light probe
        };

        enum EQuality
        {
            PX128,       //< Use cubemap resolution quality of 128px
            PX256,       //< Use cubemap resolution quality of 256px
            PX512,       //< Use cubemap resolution quality of 512px
            PX1024,      //< Use cubemap resolution quality of 1024px
            PX2048,      //< Use cubemap resolution quality of 2048px
        };

        enum ERefreshMode
        {
            Static,         //< Light will be updated at any time the settings has changed
            Dynamic,        //< Light will be updated at every frame
        };

    public:

        void SetRefreshMode(ERefreshMode _RefreshMode);
        ERefreshMode GetRefreshMode();

        void SetType(EType _Type);
        EType GetType() const;

        void SetQuality(EQuality _Quality);
        EQuality GetQuality() const;
        unsigned int GetQualityInPixel() const;

        void SetCubemap(Dt::CTextureCube* _rCubemap);
        Dt::CTextureCube* GetCubemap();

        void SetIntensity(float _Intensity);
        float GetIntensity();

    public:

        CLightProbeFacet();
        ~CLightProbeFacet();

    private:

        ERefreshMode      m_RefreshMode;        //< Refresh mode of the light probe
        EType             m_Type;               //< Type of the probe (@see EType)
        EQuality          m_Quality;            //< Quality of the probe (@see EQuality)
        Dt::CTextureCube* m_pCubemap;           //< Pointer to cube map for custom probe
        float             m_Intensity;          //< Intensity of the light probe
    };
} // namespace Dt

namespace Dt
{
    class CSkyFacet
    {
    public:

        enum EType
        {
            Procedural,      //< Sky is a procedural generated HDR depending on settings
            Panorama,        //< Sky will be created from panorama image
            Cubemap,         //< Sky is generated from given cube map
            Texture,         //< Sky is generated from given texture
        };

    public:

        void SetType(EType _Type);
        EType GetType() const;

        void SetCubemap(Dt::CTextureCube* _rCubemap);
        Dt::CTextureCube* GetCubemap();        

        void SetPanorama(Dt::CTexture2D* _pTexture2D);
        Dt::CTexture2D* GetPanorama();

        void SetTexture(Dt::CTexture2D* _pTexture2D);
        Dt::CTexture2D* GetTexture();

        bool GetHasCubemap() const;
        bool GetHasPanorama() const;
        bool GetHasTexture() const;

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

    public:

        CSkyFacet();
        ~CSkyFacet();

    private:

        EType             m_Type;               //< Type of the skybox for procedural panorama or cubemap
        bool              m_HasHDR;             //< Declares either the image consists of HDR values
        Dt::CTextureCube* m_pCubemap;           //< Pointer to cubemap for cubemap skybox
        Dt::CTexture2D*   m_pPanoramaTexture;   //< Pointer to a panorama texture
        Dt::CTexture2D*   m_pTexture;           //< Pointer to a texture
        float             m_Intensity;          //< Intensity of sky that is freely adjustable by artist (multiplier on the image)
    };
} // namespace Dt