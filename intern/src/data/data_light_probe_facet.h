
#pragma once

#include "base/base_typedef.h"
#include "base/base_vector3.h"

#include "data/data_texture_cube.h"

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
            Sky,         //< Defines a global light probe that use the sky dome
            Local,       //< Define a local light probe that captures nearby entities
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
        float GetIntensity() const;

        void SetNear(float _Near);
        float GetNear() const;

        void SetFar(float _Far);
        float GetFar() const;

        void SetParallaxCorrection(bool _Flag);
        bool GetParallaxCorrection() const;

        void SetBoxSize(const Base::Float3& _rSize);
        const Base::Float3& GetBoxSize() const;

    public:

        CLightProbeFacet();
        ~CLightProbeFacet();

    private:

        ERefreshMode      m_RefreshMode;        //< Refresh mode of the light probe
        EType             m_Type;               //< Type of the probe (@see EType)
        EQuality          m_Quality;            //< Quality of the probe (@see EQuality)
        Dt::CTextureCube* m_pCubemap;           //< Pointer to cube map for custom probe
        float             m_Intensity;          //< Intensity of the light probe
        float             m_Near;               //< Near clipping plane
        float             m_Far;                //< Far clipping plane
        bool              m_ParallaxCorrection; //< Use a parallax correction of the reflection
        Base::Float3      m_BoxSize;            //< Size of the box around the probe that affects the objects
    };
} // namespace Dt