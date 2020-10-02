
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CSkyComponent : public CComponent<CSkyComponent>
    {
    public:

        static const unsigned int s_NumberOfQualities = 6;

    public:

        enum EType
        {
            Procedural,      //< Sky is a procedural generated HDR depending on settings
            Panorama,        //< Sky will be created from panorama image
            Cubemap,         //< Sky is generated from given cube map
        };

        enum EQuality
        {
            PX64,       //< Use cubemap resolution quality of 64px
            PX128,       //< Use cubemap resolution quality of 128px
            PX256,       //< Use cubemap resolution quality of 256px
            PX512,       //< Use cubemap resolution quality of 512px
            PX1024,      //< Use cubemap resolution quality of 1024px
            PX2048,      //< Use cubemap resolution quality of 2048px
        };

        enum ERefreshMode
        {
            Static,         //< Sky will be updated at any time the settings has changed
            Dynamic,        //< Sky will be updated at every frame (can be used for dynamic images)
        };

    public:

        void SetRefreshMode(ERefreshMode _RefreshMode);
        ERefreshMode GetRefreshMode();

        void SetType(EType _Type);
        EType GetType() const;

        void SetQuality(EQuality _Quality);
        EQuality GetQuality() const;
        unsigned int GetQualityInPixel() const;

        void SetTexture(const std::string& _rTexture);
        const std::string& GetTexture() const;

        bool HasTexture() const;

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

    public:

        CSkyComponent();
        ~CSkyComponent();

    public:

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec); 

            int RefreshMode, Type, Quality;
            
            _rCodec >> RefreshMode;
            _rCodec >> Type;
            _rCodec >> Quality;
            _rCodec >> m_HasHDR;

            Base::Serialize(_rCodec, m_Texture);

            _rCodec >> m_Intensity;

            m_RefreshMode = (ERefreshMode)RefreshMode;
            m_Type = (EType)Type;
            Quality = (EQuality)Quality;
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            _rCodec << (int)m_RefreshMode;
            _rCodec << (int)m_Type;
            _rCodec << (int)m_Quality;
            _rCodec << m_HasHDR; 

            Base::Serialize(_rCodec, m_Texture);

            _rCodec << m_Intensity;
        }

        inline IComponent* Allocate() override
        {
            return new CSkyComponent();
        }

    private:

        ERefreshMode     m_RefreshMode;        //< Refresh mode of the sky
        EType            m_Type;               //< Type of the skybox for procedural panorama or cubemap
        EQuality         m_Quality;            //< Quality of the probe (@see EQuality)
        bool             m_HasHDR;             //< Declares either the image consists of HDR values
        std::string      m_Texture;            //< Texture
        float            m_Intensity;          //< Intensity of sky that is freely adjustable by artist (multiplier on the image)

    private:

        friend class CSkyComponentGUI;
    };
} // namespace Dt