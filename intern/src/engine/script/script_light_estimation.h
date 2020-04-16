
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
            Framework,
            LUT,
            Stitching,
            NumberOfEstimationTypes,
        };

    public:

        Dt::CEntity* m_pSkyEntity = nullptr;
        Dt::CSkyComponent* m_pSkyComponent = nullptr;
        Dt::CSunComponent* m_pSunComponent = nullptr;

        bool m_UseEstimationWithNeuralNetwork = false;

    private:

        using LESetInputTextureFunc = void(*)(Gfx::CTexturePtr);
        using LEGetOutputCubemapFunc = Gfx::CTexturePtr(*)();
        using LESetActiveFunc = void(*)(bool);

        using ARGetBackgroundTextureFunc = Gfx::CTexturePtr(*)();
        using ARGetLightEstimationFunc = void*(*)();
        using ARGetLightEstimationStateFunc = int(*)(void*);
        using ARGetLightEstimationMainLightIntensityFunc = glm::vec3(*)(void*);
        using ARGetLightEstimationMainLightDirectionFunc = glm::vec3(*)(void*);
        using ARGetLightEstimationHDRCubemapFunc = Gfx::CTexturePtr(*)(void*);

        ARGetLightEstimationFunc GetLightEstimation = nullptr;
        ARGetLightEstimationStateFunc GetState = nullptr;
        ARGetLightEstimationMainLightIntensityFunc GetMainLightDirection = nullptr;
        ARGetLightEstimationMainLightDirectionFunc GetMainLightIntensity = nullptr;
        ARGetLightEstimationHDRCubemapFunc GetHDRCubemap = nullptr;

        LESetInputTextureFunc SetInputTexture = nullptr;
        LEGetOutputCubemapFunc GetOutputCubemap = nullptr;
        LESetActiveFunc SetActive = nullptr;

        std::array<std::string, NumberOfEstimationTypes> m_PluginNames = { "Light Estimation Framework", "Light Estimation LUT", "Light Estimation Stitching" };

        Core::IPlugin* m_pCurrentPluginPtr = nullptr;

        int m_Mode;
        int m_SaveIndex = 0;

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
            // Prepare light estimation
            // -----------------------------------------------------------------------------
            for (auto& rPluginName : m_PluginNames) Core::PluginManager::LoadPlugin(rPluginName);
            
            SwitchLightEstimation(Framework);
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
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

                if (m_Mode == Framework)
                {
                    auto pGfxSky = static_cast<Gfx::CSky*>(m_pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

                    pGfxSky->SetInputTexture(GetHDRCubemap(pObject));
                }
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            if (_rEvent.GetAction() == Base::CInputEvent::TouchReleased)
            {
                ENGINE_CONSOLE_INFO("Touched (NE = switch estimation; SE = save cube map; NW = Estimate environment w/ NN");

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
                    m_UseEstimationWithNeuralNetwork = true;

                    ENGINE_CONSOLE_INFO("Estimate environment with NN");

                    Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*this, CLightEstimationScript::DirtyInfo);
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
                    m_UseEstimationWithNeuralNetwork = true;

                    Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*this, CLightEstimationScript::DirtyInfo);
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

            if (Core::PluginManager::LoadPlugin(CurrentPluginName))
            {
                // -----------------------------------------------------------------------------
                // Pause old plugin
                // -----------------------------------------------------------------------------
                if (SetActive != nullptr) SetActive(false);

                // -----------------------------------------------------------------------------
                // Prepare new plugin
                // -----------------------------------------------------------------------------
                SetInputTexture  = (LESetInputTextureFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetInputTexture"));
                GetOutputCubemap = (LEGetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "GetOutputCubemap"));
                SetActive        = (LESetActiveFunc)(Core::PluginManager::GetPluginFunction(CurrentPluginName, "SetActive"));

                SetActive(true);
            }

#ifdef PLATFORM_ANDROID
            std::string ARPluginName = "ArCore";
#else
            std::string ARPluginName = "EasyAR";
#endif

            if (Core::PluginManager::LoadPlugin(ARPluginName))
            {
                ARGetBackgroundTextureFunc GetBackgroundTexture;

                GetBackgroundTexture = (ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction(ARPluginName, "GetBackgroundTexture"));

                if (SetInputTexture != nullptr) SetInputTexture(GetBackgroundTexture());

                // -----------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
                GetLightEstimation = (ARGetLightEstimationFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimation"));
                GetState = (ARGetLightEstimationStateFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimationState"));
                GetMainLightDirection = (ARGetLightEstimationMainLightDirectionFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimationMainLightDirection"));
                GetMainLightIntensity = (ARGetLightEstimationMainLightIntensityFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetLightEstimationMainLightIntensity"));
                GetHDRCubemap = (ARGetLightEstimationHDRCubemapFunc)(Core::PluginManager::GetPluginFunction("ArCore", "LightEstimationGetHDRCubemap"));
#endif
            }
            else
            {
                ENGINE_CONSOLE_WARNING("No plugin is loaded to enable light stitching in AR.")
            }

			// -----------------------------------------------------------------------------
			// Setup sky
			// -----------------------------------------------------------------------------
			if (m_Mode == Framework)
			{
				Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

				m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
				m_pSkyComponent->SetTexture("");
				m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
				m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX128);
				m_pSkyComponent->SetIntensity(12000 * 3.14f);

				Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

				if (GetLightEstimation != nullptr)
				{
					void* pObject = GetLightEstimation();

					auto pGfxSky = static_cast<Gfx::CSky*>(m_pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

					pGfxSky->SetInputTexture(GetHDRCubemap(pObject));
				}
			}
			else
			{
				m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
				m_pSkyComponent->SetTexture("");
				m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
				m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX128);
				m_pSkyComponent->SetIntensity(12000);

				Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

				auto pGfxSky = static_cast<Gfx::CSky*>(m_pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

				pGfxSky->SetInputTexture(GetOutputCubemap());
			}			
        }

        // -----------------------------------------------------------------------------

        void SaveCubemap()
        {
			Gfx::CTexturePtr OutputCubemapPtr = GetOutputCubemap();

			// -----------------------------------------------------------------------------
			// Create panorama
			// -----------------------------------------------------------------------------
			Gfx::STextureDescriptor TextureDescriptor = { };

			TextureDescriptor.m_NumberOfPixelsU  = 256;
			TextureDescriptor.m_NumberOfPixelsV  = 128;
			TextureDescriptor.m_NumberOfPixelsW  = 1;
			TextureDescriptor.m_NumberOfMipMaps  = 1;
			TextureDescriptor.m_NumberOfTextures = 1;
			TextureDescriptor.m_Binding			 = Gfx::CTexture::ShaderResource;
			TextureDescriptor.m_Access			 = Gfx::CTexture::CPUWrite;
			TextureDescriptor.m_Format			 = Gfx::CTexture::R8G8B8A8_BYTE;
			TextureDescriptor.m_Usage			 = Gfx::CTexture::GPUReadWrite;
			TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;

			Gfx::CTexturePtr PanoramaTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

			std::string Define = "";
			Define += "#define TILE_SIZE 1\n";
			Define += "#define CUBE_TYPE rgba8\n";
			Define += "#define OUTPUT_TYPE rgba8\n";
			Define += "#define CUBE_SIZE " + std::to_string(OutputCubemapPtr->GetNumberOfPixelsU()) + "\n";
			Define += "#define PANORAMA_SIZE_W " + std::to_string(256) + "\n";
			Define += "#define PANORAMA_SIZE_H " + std::to_string(128) + "\n";

			Gfx::CShaderPtr C2PShaderPtr = Gfx::ShaderManager::CompileCS("helper/cs_cube2pano.glsl", "main", Define.c_str());

			Gfx::ContextManager::SetShaderCS(C2PShaderPtr);

			Gfx::ContextManager::SetImageTexture(0, OutputCubemapPtr);

			Gfx::ContextManager::SetImageTexture(1, PanoramaTexturePtr);

			Gfx::ContextManager::Dispatch(256, 128, 1);

			Gfx::ContextManager::ResetImageTexture(0);

			Gfx::ContextManager::ResetImageTexture(1);

			Gfx::ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------

            auto NameOfPano = "env_panorama_" + std::to_string(m_SaveIndex) + ".png";
            auto NameOfCube = "env_cubemap_" + std::to_string(m_SaveIndex) + ".png";

            ++ m_SaveIndex;

            // -----------------------------------------------------------------------------

            Gfx::TextureManager::SaveTexture(PanoramaTexturePtr, Core::AssetManager::GetPathToAssets() + "/" + NameOfPano);

            Gfx::TextureManager::SaveTexture(OutputCubemapPtr, Core::AssetManager::GetPathToAssets() + "/" + NameOfCube);
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