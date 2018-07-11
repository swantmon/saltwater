
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CLightEstimationScript : public CScript<CLightEstimationScript>
    {
    public:

        enum EEstimationType
        {
            Stitching,
            LUT,
            NumberOfEstimationTypes,
        };

    public:

        Dt::CEntity* m_pSkyEntity = nullptr;
        Dt::CSkyComponent* m_pSkyComponent = nullptr;

    private:

        typedef void(*LESetInputTextureFunc)(Gfx::CTexturePtr);
        typedef void(*LESetOutputCubemapFunc)(Gfx::CTexturePtr);
        typedef Gfx::CTexturePtr(*LEGetOutputCubemapFunc)();
        typedef Gfx::CTexturePtr(*ARGetBackgroundTextureFunc)();

        LESetInputTextureFunc SetInputTexture;
        LESetOutputCubemapFunc SetOutputCubemap;
        LEGetOutputCubemapFunc GetOutputCubemap;
        ARGetBackgroundTextureFunc GetBackgroundTexture;

        Gfx::CTexturePtr m_OutputCubemapPtr;

        std::string m_PluginNames[NumberOfEstimationTypes] = { "Light Estimation Stitching", "Light Estimation LUT" };

        Core::IPlugin* m_pCurrentPluginPtr = nullptr;

        int m_Mode;

    public:

        void Start() override
        {
            // -----------------------------------------------------------------------------
            // Prepare sky entity
            // -----------------------------------------------------------------------------
            m_pSkyEntity = GetEntity();

            if (m_pSkyEntity != nullptr)
            {
                m_pSkyComponent = m_pSkyEntity->GetComponentFacet()->GetComponent<Dt::CSkyComponent>();
            }

            if (m_pSkyComponent == nullptr) return;

            // -----------------------------------------------------------------------------
            // Input
            // -----------------------------------------------------------------------------
            if (Core::PluginManager::HasPlugin("ArCore"))
            {
                GetBackgroundTexture = (ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetBackgroundTexture"));
            }
            else if (Core::PluginManager::HasPlugin("EasyAR"))
            {
                GetBackgroundTexture = (ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "GetBackgroundTexture"));
            }
            else
            {
//                 Gfx::STextureDescriptor TextureDescriptor;
// 
//                 TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;
//                 TextureDescriptor.m_NumberOfPixelsU  = Gfx::STextureDescriptor::s_FormatFromSource;
//                 TextureDescriptor.m_NumberOfPixelsV  = Gfx::STextureDescriptor::s_FormatFromSource;
//                 TextureDescriptor.m_NumberOfPixelsW  = 1;
//                 TextureDescriptor.m_NumberOfTextures = 1;
//                 TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
//                 TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
//                 TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
//                 TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
//                 TextureDescriptor.m_pFileName        = "environments/Lobby-Center_2k.hdr";
//                 TextureDescriptor.m_pPixels          = 0;
// 
//                 SetInputTexture(Gfx::TextureManager::CreateTexture2D(TextureDescriptor));

                return;
            }

            // -----------------------------------------------------------------------------
            // Output cube map texture
            // -----------------------------------------------------------------------------
            Gfx::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = 512;
            TextureDescriptor.m_NumberOfPixelsV  = 512;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
            TextureDescriptor.m_NumberOfTextures = 6;
            TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource | Gfx::CTexture::RenderTarget;
            TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
            TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
            TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
            TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
            TextureDescriptor.m_pFileName        = 0;
            TextureDescriptor.m_pPixels          = 0;
            TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_BYTE;

            m_OutputCubemapPtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);

            Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Sky cubemap from image");

            // -----------------------------------------------------------------------------
            // Setup sky
            // -----------------------------------------------------------------------------
            m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
            m_pSkyComponent->SetTexture(m_OutputCubemapPtr);
            m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
            m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX128);
            m_pSkyComponent->SetIntensity(12000);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

            // -----------------------------------------------------------------------------
            // Prepare light estimation
            // -----------------------------------------------------------------------------
            SwitchLightEstimation(Stitching);
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            m_OutputCubemapPtr = 0;
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            // -----------------------------------------------------------------------------
            // Switch mode
            // -----------------------------------------------------------------------------
            if (_rEvent.GetAction() == Base::CInputEvent::TouchReleased)
            {
                float x = _rEvent.GetCursorPosition()[0];
                float y = _rEvent.GetCursorPosition()[1];

                if (x < 200.0f && y < 200.0f)
                {
                    m_Mode = (m_Mode + 1) % NumberOfEstimationTypes;

                    SwitchLightEstimation((EEstimationType)m_Mode);
                }
            }

            if (_rEvent.GetAction() == Base::CInputEvent::KeyReleased && _rEvent.GetKey() == Base::CInputEvent::Key0)
            {
                m_Mode = (m_Mode + 1) % NumberOfEstimationTypes;

                SwitchLightEstimation((EEstimationType)m_Mode);
            }

            // -----------------------------------------------------------------------------
            // Save textures
            // -----------------------------------------------------------------------------
            if (_rEvent.GetAction() == Base::CInputEvent::TouchReleased)
            {
                float x = _rEvent.GetCursorPosition()[0];
                float y = _rEvent.GetCursorPosition()[1];

                if (x > Gfx::Main::GetActiveNativeWindowSize()[0] - 200.0f && y > Gfx::Main::GetActiveNativeWindowSize()[1] - 200.0f)
                {
                    SaveCubemap();
                }
            }

            if (_rEvent.GetAction() == Base::CInputEvent::KeyReleased && _rEvent.GetKey() == Base::CInputEvent::Key1)
            {
                SaveCubemap();
            }
        }

        // -----------------------------------------------------------------------------

        void SwitchLightEstimation(EEstimationType _EstimationType)
        {
            m_Mode = _EstimationType;

            // -----------------------------------------------------------------------------
            // Is plugin available/loaded?
            // -----------------------------------------------------------------------------
            std::string CurrentPluginName = m_PluginNames[_EstimationType];

            if (!Core::PluginManager::HasPlugin(CurrentPluginName))
            {
                return;
            }

            // -----------------------------------------------------------------------------
            // Pause old plugin
            // -----------------------------------------------------------------------------
            if (m_pCurrentPluginPtr) m_pCurrentPluginPtr->OnPause();

            // -----------------------------------------------------------------------------
            // Prepare new plugin
            // -----------------------------------------------------------------------------
            SetInputTexture  = (LESetInputTextureFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetInputTexture"));
            SetOutputCubemap = (LESetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetOutputCubemap"));
            GetOutputCubemap = (LEGetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "GetOutputCubemap"));

            m_pCurrentPluginPtr = Core::PluginManager::GetPlugin(CurrentPluginName);

            m_pCurrentPluginPtr->OnResume();

            SetInputTexture(GetBackgroundTexture());

            SetOutputCubemap(m_OutputCubemapPtr);
        }

        // -----------------------------------------------------------------------------

        void SaveCubemap()
        {
            Gfx::TextureManager::SaveTexture(m_OutputCubemapPtr, Core::AssetManager::GetPathToFiles() + "/env_cubemap.ppm");
        }
    };
} // namespace Scpt