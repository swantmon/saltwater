
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"
#include "base/base_serialize_glm.h"
#include "base/base_typedef.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CLightProbeComponent : public CComponent<CLightProbeComponent>
    {
    public:

        static const unsigned int s_NumberOfQualities = 7;

    public:

        enum EType
        {
            Sky,         //< Defines a light probe that use the sky box
            Local,       //< Define a local light probe that captures nearby entities and sky box
            Custom,      //< A cubemap texture can be set for generating the global light probe
        };

        enum EQuality
        {
            PX32,        //< Use cubemap resolution quality of 32px
            PX64,        //< Use cubemap resolution quality of 64px
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

        enum EClearFlag
        {
            Skybox,
            Black
        };

    public:

        void SetRefreshMode(ERefreshMode _RefreshMode);
        ERefreshMode GetRefreshMode();

        void SetType(EType _Type);
        EType GetType() const;

        void SetQuality(EQuality _Quality);
        EQuality GetQuality() const;
        unsigned int GetQualityInPixel() const;

        void SetClearFlag(EClearFlag _ClearFlag);
        EClearFlag GetClearFlag() const;

        void SetCubemap(const std::string& _rCubemap);
        const std::string& GetCubemap();

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

        void SetNear(float _Near);
        float GetNear() const;

        void SetFar(float _Far);
        float GetFar() const;

        void SetParallaxCorrection(bool _Flag);
        bool GetParallaxCorrection() const;

        void SetBoxSize(const glm::vec3& _rSize);
        const glm::vec3& GetBoxSize() const;

    public:

        CLightProbeComponent();
        ~CLightProbeComponent();

    public:

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            int RefreshMode;
            int Type;
            int Quality;
            int ClearFlag;

            _rCodec >> RefreshMode;
            _rCodec >> Type;
            _rCodec >> Quality;
            _rCodec >> ClearFlag;

            m_RefreshMode = (ERefreshMode)RefreshMode;
            m_Type = (EType)Type;
            m_Quality = (EQuality)Quality;
            m_ClearFlag = (EClearFlag)ClearFlag;

            Base::Serialize(_rCodec, m_Cubemap);

            _rCodec >> m_Intensity;
            _rCodec >> m_Near;
            _rCodec >> m_Far;
            _rCodec >> m_ParallaxCorrection;

            Base::Serialize(_rCodec, m_BoxSize);
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            _rCodec << (int)m_RefreshMode;        
            _rCodec << (int)m_Type;               
            _rCodec << (int)m_Quality;            
            _rCodec << (int)m_ClearFlag;          

            Base::Serialize(_rCodec, m_Cubemap);            

            _rCodec << m_Intensity;          
            _rCodec << m_Near;               
            _rCodec << m_Far;                
            _rCodec << m_ParallaxCorrection; 

            Base::Serialize(_rCodec, m_BoxSize);
        }

        inline IComponent* Allocate() override
        {
            return new CLightProbeComponent();
        }

    private:

        ERefreshMode m_RefreshMode;        //< Refresh mode of the light probe
        EType        m_Type;               //< Type of the probe (@see EType)
        EQuality     m_Quality;            //< Quality of the probe (@see EQuality)
        EClearFlag   m_ClearFlag;          //< Clear flag of the reflection probe (@see EClearFlag)
        std::string  m_Cubemap;            //< Name of the cube map for custom probe
        float        m_Intensity;          //< Intensity of the light probe
        float        m_Near;               //< Near clipping plane
        float        m_Far;                //< Far clipping plane
        bool         m_ParallaxCorrection; //< Use a parallax correction of the reflection
        glm::vec3    m_BoxSize;            //< Size of the box around the probe that affects the objects

    private:

        friend class CLightProbeComponentGUI;
    };
} // namespace Dt