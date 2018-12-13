
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

        static const unsigned int s_PanoramaWidth  = 256;
        static const unsigned int s_PanoramaHeight = 128;
        static const unsigned int s_CubemapSize    = 512;

    private:

        typedef void(*LESetInputTextureFunc)(Gfx::CTexturePtr);
        typedef void(*LESetOutputCubemapFunc)(Gfx::CTexturePtr);
        typedef Gfx::CTexturePtr(*LEGetOutputCubemapFunc)();
        typedef Gfx::CTexturePtr(*ARGetBackgroundTextureFunc)();

        LESetInputTextureFunc SetInputTexture;
        LESetOutputCubemapFunc SetOutputCubemap;
        LEGetOutputCubemapFunc GetOutputCubemap;

        Gfx::CShaderPtr m_C2PShaderPtr;
        Gfx::CShaderPtr m_FusePanoramaShaderPtr;

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
            std::string Define = "";
            Define += "#define TILE_SIZE 1\n";
            Define += "#define CUBE_TYPE rgba8\n";
            Define += "#define OUTPUT_TYPE rgba8\n";
            Define += "#define CUBE_SIZE " + std::to_string(s_CubemapSize) + "\n";
            Define += "#define PANORAMA_SIZE_W " + std::to_string(s_PanoramaWidth) + "\n";
            Define += "#define PANORAMA_SIZE_H " + std::to_string(s_PanoramaHeight) + "\n";

            m_C2PShaderPtr = Gfx::ShaderManager::CompileCS("helper/cs_cube2pano.glsl", "main", Define.c_str());

            Define = "";
            Define += "#define TILE_SIZE 1\n";
            Define += "#define PANORAMA_TYPE rgba8\n";

            m_FusePanoramaShaderPtr = Gfx::ShaderManager::CompileCS("helper/cs_fusepano.glsl", "main", Define.c_str());

            // -----------------------------------------------------------------------------
            // Output cube map texture
            // -----------------------------------------------------------------------------
            ResetTextures();

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
        }

        // -----------------------------------------------------------------------------

        void OnNewMessage(const Net::CMessage& _rMessage, int _Port)
        {
            BASE_UNUSED(_Port);

            if (_rMessage.m_DecompressedSize != s_PanoramaWidth * s_PanoramaHeight * 4) return;

            // -----------------------------------------------------------------------------
            // Read new texture from network
            // -----------------------------------------------------------------------------
            Gfx::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = s_PanoramaWidth;
            TextureDescriptor.m_NumberOfPixelsV  = s_PanoramaHeight;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_NumberOfMipMaps  = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
            TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
            TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;
            TextureDescriptor.m_Usage            = Gfx::CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
            TextureDescriptor.m_pFileName        = 0;
            TextureDescriptor.m_pPixels          = const_cast<char*>(&_rMessage.m_Payload[0]);
        
            Gfx::CTexturePtr NewTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            // -----------------------------------------------------------------------------
            // Create combination of new texture and existing data
            // -----------------------------------------------------------------------------
            Gfx::ContextManager::SetShaderCS(m_FusePanoramaShaderPtr);

            Gfx::ContextManager::SetImageTexture(0, NewTexturePtr);

            Gfx::ContextManager::SetImageTexture(1, m_PanoramaTexturePtr);

            Gfx::ContextManager::SetImageTexture(2, m_PanoramaTexturePtr);

            Gfx::ContextManager::Dispatch(s_PanoramaWidth, s_PanoramaHeight, 1);

            Gfx::ContextManager::ResetImageTexture(0);

            Gfx::ContextManager::ResetImageTexture(1);

            Gfx::ContextManager::ResetImageTexture(2);

            Gfx::ContextManager::ResetShaderCS();

            NewTexturePtr = 0;

            // -----------------------------------------------------------------------------
            // Set new texture to sky
            // -----------------------------------------------------------------------------
            Gfx::TextureManager::SetTextureLabel(m_PanoramaTexturePtr, "Sky panorama from image");

            m_pSkyComponent->SetType(Dt::CSkyComponent::Panorama);
            m_pSkyComponent->SetTexture(m_PanoramaTexturePtr);
            m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
            m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX256);
            m_pSkyComponent->SetIntensity(12000);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            if (_rEvent.GetAction() == Base::CInputEvent::TouchReleased)
            {
                ENGINE_CONSOLE_INFO("Touched (NE = switch estimation; SE = save cube map; NW = send panorama; SW = reset");

                float x = _rEvent.GetGlobalCursorPosition()[0];
                float y = _rEvent.GetGlobalCursorPosition()[1];

                if (x < 200.0f && y < 200.0f)
                {
                    ENGINE_CONSOLE_INFO("Switch light estimation");

                    m_Mode = (m_Mode + 1) % NumberOfEstimationTypes;

                    SwitchLightEstimation((EEstimationType)m_Mode);
                }

                if (x < 200.0f && y > Gfx::Main::GetActiveNativeWindowSize()[1] - 200.0f)
                {
                    ENGINE_CONSOLE_INFO("Save cubemap");

                    SaveCubemap();
                }

                if (x > Gfx::Main::GetActiveNativeWindowSize()[0] - 200.0f && y < 200.0f)
                {
                    if (m_OutputCubemapPtr != nullptr && Net::CNetworkManager::GetInstance().IsConnected())
                    {
                        ENGINE_CONSOLE_INFO("Send panorama via network");

                        SendPanoramaTexture();
                    }
                    else
                    {
                        ENGINE_CONSOLE_INFO("No connection between script and endpoint. Please check connection.");
                    }
                }

                if (x > Gfx::Main::GetActiveNativeWindowSize()[0] - 200.0f && y > Gfx::Main::GetActiveNativeWindowSize()[1] - 200.0f)
                {
                    ENGINE_CONSOLE_INFO("Reset panorama texture");

                    ResetTextures();
                }
            }

            // -----------------------------------------------------------------------------

            if (_rEvent.GetAction() == Base::CInputEvent::KeyReleased)
            {
                switch (_rEvent.GetKey())
                {
                case Base::CInputEvent::Key0:
                    m_Mode = (m_Mode + 1) % NumberOfEstimationTypes;

                    SwitchLightEstimation((EEstimationType)m_Mode);
                    break;
                case Base::CInputEvent::Key1:
                    SaveCubemap();
                    break;
                case Base::CInputEvent::Key2:
                    if (m_OutputCubemapPtr != nullptr && Net::CNetworkManager::GetInstance().IsConnected())
                    {
                        SendPanoramaTexture();
                    }
                    break;
                case Base::CInputEvent::Key3:
                    ResetTextures();
                    break;
                };
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
                ENGINE_CONSOLE_WARNING("No plugin is loaded to enable light stitching in AR.")

//                 Gfx::STextureDescriptor TextureDescriptor;
// 
//                 TextureDescriptor.m_Format           = Gfx::STextureDescriptor::s_FormatFromSource;
//                 TextureDescriptor.m_NumberOfPixelsU  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
//                 TextureDescriptor.m_NumberOfPixelsV  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
//                 TextureDescriptor.m_NumberOfPixelsW  = 1;
//                 TextureDescriptor.m_NumberOfTextures = 1;
//                 TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
//                 TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
//                 TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
//                 TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
//                 TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
//                 TextureDescriptor.m_pFileName        = "environments/Lobby-Center_2k.hdr";
//                 TextureDescriptor.m_pPixels          = 0;
// 
//                 SetInputTexture(Gfx::TextureManager::CreateTexture2D(TextureDescriptor));
            }
        }

        // -----------------------------------------------------------------------------

        void SaveCubemap()
        {
            Gfx::ContextManager::SetShaderCS(m_C2PShaderPtr);

            Gfx::ContextManager::SetImageTexture(0, m_OutputCubemapPtr);

            Gfx::ContextManager::SetImageTexture(1, m_PanoramaTexturePtr);

            Gfx::ContextManager::Dispatch(s_PanoramaWidth, s_PanoramaHeight, 1);

            Gfx::ContextManager::ResetImageTexture(0);

            Gfx::ContextManager::ResetImageTexture(1);

            Gfx::ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------

            Gfx::TextureManager::SaveTexture(m_PanoramaTexturePtr, Core::AssetManager::GetPathToFiles() + "/env_panorama.ppm");

            Gfx::TextureManager::SaveTexture(m_OutputCubemapPtr, Core::AssetManager::GetPathToFiles() + "/env_cubemap.ppm");
        }

        // -----------------------------------------------------------------------------

        void SendPanoramaTexture()
        {
            Gfx::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = s_PanoramaWidth;
            TextureDescriptor.m_NumberOfPixelsV  = s_PanoramaHeight;
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

            Gfx::ContextManager::SetShaderCS(m_C2PShaderPtr);

            Gfx::ContextManager::SetImageTexture(0, m_OutputCubemapPtr);

            Gfx::ContextManager::SetImageTexture(1, m_PanoramaTexturePtr);

            Gfx::ContextManager::Dispatch(s_PanoramaWidth, s_PanoramaHeight, 1);

            Gfx::ContextManager::ResetImageTexture(0);

            Gfx::ContextManager::ResetImageTexture(1);

            Gfx::ContextManager::ResetShaderCS();

            std::vector<char> Data(s_PanoramaWidth * s_PanoramaHeight * 4);

            Gfx::TextureManager::CopyTextureToCPU(m_PanoramaTexturePtr, Data.data());

            Net::CNetworkManager::GetInstance().SendMessage(0, Data);
        }

        // -----------------------------------------------------------------------------

        void ResetTextures()
        {
            Gfx::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = s_PanoramaWidth;
            TextureDescriptor.m_NumberOfPixelsV  = s_PanoramaHeight;
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

            TextureDescriptor.m_NumberOfPixelsU  = s_CubemapSize;
            TextureDescriptor.m_NumberOfPixelsV  = s_CubemapSize;
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

            Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Sky cube map from image");

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

            SwitchLightEstimation((EEstimationType)m_Mode);
        }
    };
} // namespace Scpt