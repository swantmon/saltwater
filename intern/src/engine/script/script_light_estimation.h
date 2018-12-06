
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "engine/network/core_network_manager.h"

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

        Gfx::CShaderPtr m_C2PShaderPtr;

        Gfx::CTexturePtr m_OutputCubemapPtr;
        Gfx::CTexturePtr m_PanoramaTexturePtr;

        std::string m_PluginNames[NumberOfEstimationTypes] = { "Light Estimation Stitching", "Light Estimation LUT" };

        Core::IPlugin* m_pCurrentPluginPtr = nullptr;

        int m_Mode;

        std::shared_ptr<Net::CMessageDelegate> m_NetworkDelegate;

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
            // Shader
            // -----------------------------------------------------------------------------
            m_C2PShaderPtr = Gfx::ShaderManager::CompileCS("helper/cs_cube2pano.glsl", "main", "\
                #define TILE_SIZE 1\n \
                #define CUBE_TYPE rgba8\n \
                #define OUTPUT_TYPE rgba8\n \
                #define CUBE_SIZE 512\n");

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
        
            TextureDescriptor.m_NumberOfPixelsU  = 128;
            TextureDescriptor.m_NumberOfPixelsV  = 64;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_NumberOfMipMaps  = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
            TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
            TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_BYTE;
            TextureDescriptor.m_Usage            = Gfx::CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
            TextureDescriptor.m_pFileName        = 0;
            TextureDescriptor.m_pPixels          = 0;
        
            m_PanoramaTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            Gfx::TextureManager::SetTextureLabel(m_PanoramaTexturePtr, "Sky panorama from image");

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

            m_NetworkDelegate = std::shared_ptr<Net::CMessageDelegate>(new Net::CMessageDelegate(std::bind(&CLightEstimationScript::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2)));
            Net::CNetworkManager::GetInstance().RegisterMessageHandler(0, m_NetworkDelegate);
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            m_OutputCubemapPtr = 0;
            m_C2PShaderPtr = 0;
            m_PanoramaTexturePtr = 0;
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (m_OutputCubemapPtr != nullptr && Net::CNetworkManager::GetInstance().IsConnected())
            {
                SendPanoramaTexture();
            }
        }

        // -----------------------------------------------------------------------------

        void OnNewMessage(const Net::CMessage& _rMessage, int _Port)
        {
            BASE_UNUSED(_Port);

            float x = 0;
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            // -----------------------------------------------------------------------------
            // Switch mode
            // -----------------------------------------------------------------------------
            if (_rEvent.GetAction() == Base::CInputEvent::TouchReleased)
            {
                float x = _rEvent.GetGlobalCursorPosition()[0];
                float y = _rEvent.GetGlobalCursorPosition()[1];

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
                float x = _rEvent.GetGlobalCursorPosition()[0];
                float y = _rEvent.GetGlobalCursorPosition()[1];

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

            SetOutputCubemap(m_OutputCubemapPtr);

            if (Core::PluginManager::HasPlugin("ArCore") || Core::PluginManager::HasPlugin("EasyAR"))
            {
                ARGetBackgroundTextureFunc GetBackgroundTexture;

                if (Core::PluginManager::HasPlugin("ArCore"))
                {
                    GetBackgroundTexture = (ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetBackgroundTexture"));
                }
                else
                {
                    GetBackgroundTexture = (ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "GetBackgroundTexture"));
                }

                SetInputTexture(GetBackgroundTexture());
            }
            else
            {
                Gfx::STextureDescriptor TextureDescriptor;

                TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;
                TextureDescriptor.m_NumberOfPixelsU  = Gfx::STextureDescriptor::s_FormatFromSource;
                TextureDescriptor.m_NumberOfPixelsV  = Gfx::STextureDescriptor::s_FormatFromSource;
                TextureDescriptor.m_NumberOfPixelsW  = 1;
                TextureDescriptor.m_NumberOfTextures = 1;
                TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
                TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
                TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
                TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
                TextureDescriptor.m_pFileName        = "environments/Lobby-Center_2k.hdr";
                TextureDescriptor.m_pPixels          = 0;

                SetInputTexture(Gfx::TextureManager::CreateTexture2D(TextureDescriptor));
            }
        }

        // -----------------------------------------------------------------------------

        void SaveCubemap()
        {
            Gfx::ContextManager::SetShaderCS(m_C2PShaderPtr);

            Gfx::ContextManager::SetImageTexture(0, static_cast<Gfx::CTexturePtr>(m_OutputCubemapPtr));

            Gfx::ContextManager::SetImageTexture(1, static_cast<Gfx::CTexturePtr>(m_PanoramaTexturePtr));

            Gfx::ContextManager::Dispatch(128, 64, 1);

            Gfx::ContextManager::ResetImageTexture(0);

            Gfx::ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------

            Gfx::TextureManager::SaveTexture(m_PanoramaTexturePtr, Core::AssetManager::GetPathToFiles() + "/env_panorama.ppm");

            Gfx::TextureManager::SaveTexture(m_OutputCubemapPtr, Core::AssetManager::GetPathToFiles() + "/env_cubemap.ppm");
        }

        // -----------------------------------------------------------------------------

        void SendPanoramaTexture()
        {
            Gfx::ContextManager::SetShaderCS(m_C2PShaderPtr);

            Gfx::ContextManager::SetImageTexture(0, static_cast<Gfx::CTexturePtr>(m_OutputCubemapPtr));

            Gfx::ContextManager::SetImageTexture(1, static_cast<Gfx::CTexturePtr>(m_PanoramaTexturePtr));

            Gfx::ContextManager::Dispatch(128, 64, 1);

            Gfx::ContextManager::ResetImageTexture(0);

            Gfx::ContextManager::ResetShaderCS();

            std::vector<char> Data(128 * 64 * 4);

            Gfx::TextureManager::CopyTextureToCPU(m_PanoramaTexturePtr, Data.data());

            Net::CNetworkManager::GetInstance().SendMessage(0, Data);
        }
    };
} // namespace Scpt