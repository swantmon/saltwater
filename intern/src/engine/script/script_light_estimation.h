
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_plugin_manager.h"

#include "engine/core/core_program_parameters.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include <engine/data/data_sun_component.h>
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_sky.h"
#include "engine/graphic/gfx_sky_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "engine/network/core_network_manager.h"

#include "engine/script/script_script.h"

#include <array>
#include <string>

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
        Dt::CSunComponent* m_pSunComponent = nullptr;

    private:

        static const unsigned int s_PanoramaWidth  = 256;
        static const unsigned int s_PanoramaHeight = 128;
        static const unsigned int s_CubemapSize    = 512;

    private:

        using LESetInputTextureFunc = void(*)(Gfx::CTexturePtr);
        using LESetOutputCubemapFunc = void(*)(Gfx::CTexturePtr);
        using LEGetOutputCubemapFunc = Gfx::CTexturePtr(*)();
        using LESetActiveFunc = void(*)(bool);

        using ARGetBackgroundTextureFunc = Gfx::CTexturePtr(*)();
        using ARGetLightEstimationFunc = void*(*)();
        using ARGetLightEstimationStateFunc = int(*)(void*);
        using ARGetLightEstimationMainLightIntensityFunc = glm::vec3(*)(void*);
        using ARGetLightEstimationMainLightDirectionFunc = glm::vec3(*)(void*);
        using ARLightEstimationGetHDRCubemapFunc = Gfx::CTexturePtr(*)(void*);

        ARGetLightEstimationFunc GetLightEstimation = nullptr;
        ARGetLightEstimationStateFunc GetState = nullptr;
        ARGetLightEstimationMainLightIntensityFunc GetMainLightDirection = nullptr;
        ARGetLightEstimationMainLightDirectionFunc GetMainLightIntensity = nullptr;

        ARLightEstimationGetHDRCubemapFunc GetHDRCubemap = nullptr;

        LESetInputTextureFunc SetInputTexture = nullptr;
        LESetOutputCubemapFunc SetOutputCubemap = nullptr;
        LEGetOutputCubemapFunc GetOutputCubemap = nullptr;
        LESetActiveFunc SetActive = nullptr;

        Gfx::CShaderPtr m_C2PShaderPtr;
        Gfx::CShaderPtr m_FusePanoramaShaderPtr;

        Gfx::CTexturePtr m_OutputCubemapPtr;
        Gfx::CTexturePtr m_ArCoreOutputCubemapPtr;
        Gfx::CTexturePtr m_PanoramaTexturePtr;

        std::array<std::string, NumberOfEstimationTypes> m_PluginNames = { "Light Estimation Stitching", "Light Estimation LUT" };

        Core::IPlugin* m_pCurrentPluginPtr = nullptr;

        int m_Mode;

        Net::CNetworkManager::CMessageDelegate::HandleType m_NetworkDelegate;
        Net::SocketHandle m_SocketHandle;

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
                m_pSunComponent = m_pSkyEntity->GetComponentFacet()->GetComponent<Dt::CSunComponent>();
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

            Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Sky cube map from image");

            // -----------------------------------------------------------------------------
            // Prepare light estimation
            // -----------------------------------------------------------------------------
            for (auto& rPluginName : m_PluginNames) Core::PluginManager::LoadPlugin(rPluginName);
            
            SwitchLightEstimation(Stitching);

            std::string IP = Core::CProgramParameters::GetInstance().Get("mr:stitching:server_ip", "127.0.0.1");
            int Port = Core::CProgramParameters::GetInstance().Get("mr:stitching:network_port", 12345);

            m_SocketHandle = Net::CNetworkManager::GetInstance().CreateClientSocket(IP, Port);
            m_NetworkDelegate = Net::CNetworkManager::GetInstance().RegisterMessageHandler(m_SocketHandle, std::bind(&CLightEstimationScript::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            m_OutputCubemapPtr = nullptr;
            m_C2PShaderPtr = nullptr;
            m_PanoramaTexturePtr = nullptr;


            m_ArCoreOutputCubemapPtr = nullptr;
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (Core::PluginManager::IsAvailable("ArCore"))
            {
                void* pObject = GetLightEstimation();

                if (pObject == nullptr) return;

                int State = GetState(pObject);

                if (State != 1) return;

                // -----------------------------------------------------------------------------

                glm::vec3 MainLightDirection = GetMainLightDirection(pObject);

                glm::vec3 MainLightIntensity = GetMainLightIntensity(pObject);

                if (m_pSunComponent == nullptr) return;

                m_pSunComponent->SetDirection(-MainLightDirection);

                m_pSunComponent->SetColor(glm::normalize(MainLightIntensity));

                m_pSunComponent->SetIntensity(glm::length(MainLightIntensity) * 90600.0f);

                m_pSunComponent->UpdateLightness();

                // -----------------------------------------------------------------------------

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

                auto pGfxSky = static_cast<Gfx::CSky*>(m_pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

                pGfxSky->SetInputTexture(GetHDRCubemap(pObject));
            }
        }

        // -----------------------------------------------------------------------------

        void OnNewMessage(const Net::CMessage& _rMessage, Net::SocketHandle _SocketHandle)
        {
            BASE_UNUSED(_SocketHandle);

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
            TextureDescriptor.m_pFileName        = nullptr;
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

            NewTexturePtr = nullptr;

            // -----------------------------------------------------------------------------
            // Set new texture to sky
            // -----------------------------------------------------------------------------
            Gfx::TextureManager::SetTextureLabel(m_PanoramaTexturePtr, "Sky panorama from image");

            m_pSkyComponent->SetType(Dt::CSkyComponent::Panorama);
            m_pSkyComponent->SetTexture("");
            m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
            m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX256);
            m_pSkyComponent->SetIntensity(12000);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

            // -----------------------------------------------------------------------------

            auto pGfxSky = static_cast<Gfx::CSky*>(m_pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

            pGfxSky->SetInputTexture(m_PanoramaTexturePtr);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            if (_rEvent.GetAction() == Base::CInputEvent::TouchReleased)
            {
                ENGINE_CONSOLE_INFO("Touched (NE = switch estimation; SE = save cube map; NW = send panorama; SW = reset");

                auto x = static_cast<float>(_rEvent.GetGlobalCursorPosition()[0]);
                auto y = static_cast<float>(_rEvent.GetGlobalCursorPosition()[1]);

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
                    if (m_OutputCubemapPtr != nullptr && Net::CNetworkManager::GetInstance().IsConnected(m_SocketHandle))
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
                    if (m_OutputCubemapPtr != nullptr && Net::CNetworkManager::GetInstance().IsConnected(m_SocketHandle))
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

            if (!Core::PluginManager::LoadPlugin(CurrentPluginName)) return;

            // -----------------------------------------------------------------------------
            // Pause old plugin
            // -----------------------------------------------------------------------------
            if (SetActive != nullptr) SetActive(false);

            // -----------------------------------------------------------------------------
            // Prepare new plugin
            // -----------------------------------------------------------------------------
            SetInputTexture  = (LESetInputTextureFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetInputTexture"));
            SetOutputCubemap = (LESetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetOutputCubemap"));
            GetOutputCubemap = (LEGetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "GetOutputCubemap"));
            SetActive        = (LESetActiveFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetActive"));

            SetActive(true);

            SetOutputCubemap(m_OutputCubemapPtr);

#ifdef PLATFORM_ANDROID
            std::string ARPluginName = "ArCore";
#else
            std::string ARPluginName = "EasyAR";
#endif

            if (Core::PluginManager::LoadPlugin(ARPluginName))
            {
                ARGetBackgroundTextureFunc GetBackgroundTexture;

                GetBackgroundTexture = (ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction(ARPluginName, "GetBackgroundTexture"));

                SetInputTexture(GetBackgroundTexture());

                // -----------------------------------------------------------------------------

                GetLightEstimation = (ARGetLightEstimationFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimation"));
                GetState = (ARGetLightEstimationStateFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimationState"));
                GetMainLightDirection = (ARGetLightEstimationMainLightDirectionFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimationMainLightDirection"));
                GetMainLightIntensity = (ARGetLightEstimationMainLightIntensityFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimationMainLightIntensity"));
                GetHDRCubemap = (ARLightEstimationGetHDRCubemapFunc)(Core::PluginManager::GetPluginFunction("ArCore", "LightEstimationGetHDRCubemap"));
            }
            else
            {
                ENGINE_CONSOLE_WARNING("No plugin is loaded to enable light stitching in AR.")
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

            Gfx::TextureManager::SaveTexture(m_PanoramaTexturePtr, Core::AssetManager::GetPathToAssets() + "/env_panorama.png");

            Gfx::TextureManager::SaveTexture(m_OutputCubemapPtr, Core::AssetManager::GetPathToAssets() + "/env_cubemap.png");
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
            TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;
            TextureDescriptor.m_Usage            = Gfx::CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
            TextureDescriptor.m_pFileName        = nullptr;
            TextureDescriptor.m_pPixels          = nullptr;

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

            Net::CMessage Message;

            Message.m_Payload = std::vector<char>(s_PanoramaWidth * s_PanoramaHeight * 4);
            Message.m_CompressedSize = Message.m_DecompressedSize = static_cast<int>(Message.m_Payload.size());
            Message.m_MessageType = 0;

            Gfx::TextureManager::CopyTextureToCPU(m_PanoramaTexturePtr, Message.m_Payload.data());

            Net::CNetworkManager::GetInstance().SendMessage(m_SocketHandle, Message);
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
            TextureDescriptor.m_pFileName        = nullptr;
            TextureDescriptor.m_pPixels          = nullptr;
        
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
            TextureDescriptor.m_pFileName        = nullptr;
            TextureDescriptor.m_pPixels          = nullptr;
            TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_BYTE;

            m_OutputCubemapPtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);

            Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Sky cube map from image");

            // -----------------------------------------------------------------------------
            // Setup sky
            // -----------------------------------------------------------------------------
            m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
            m_pSkyComponent->SetTexture("");
            m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
            m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX128);
            m_pSkyComponent->SetIntensity(12000);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

            // -----------------------------------------------------------------------------

            auto pGfxSky = static_cast<Gfx::CSky*>(m_pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

            pGfxSky->SetInputTexture(m_OutputCubemapPtr);

            // -----------------------------------------------------------------------------

            SwitchLightEstimation((EEstimationType)m_Mode);
        }

    public:

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CLightEstimationScript();
        }
    };
} // namespace Scpt