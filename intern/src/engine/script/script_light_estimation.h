
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CLightEstimationScript : public CScript<CLightEstimationScript>
    {
    public:

        typedef void (*LightEstimationLUTSetInputTextureFunc)(Gfx::CTexturePtr);
        typedef Gfx::CTexturePtr (*LightEstimationLUTGetOutputCubemapFunc)();
        typedef Gfx::CTexturePtr (*ArCoreGetBackgroundTextureFunc)();

        LightEstimationLUTSetInputTextureFunc SetInputTexture;
        LightEstimationLUTGetOutputCubemapFunc GetOutputCubemap;
        ArCoreGetBackgroundTextureFunc GetBackgroundTexture;

    public:

        Dt::CEntity* m_pSkyEntity = nullptr;
        Dt::CSkyComponent* m_pSkyComponent = nullptr;

    public:

        void Start() override
        {
            m_pSkyEntity = GetEntity();

            if (m_pSkyEntity != nullptr)
            {
                m_pSkyComponent = m_pSkyEntity->GetComponentFacet()->GetComponent<Dt::CSkyComponent>();
            }

            if (m_pSkyComponent == nullptr) return;

            SetInputTexture      = (LightEstimationLUTSetInputTextureFunc)(Core::PluginManager::GetPluginFunction("Light Estimation LUT", "SetInputTexture"));
            GetOutputCubemap     = (LightEstimationLUTGetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction("Light Estimation LUT", "GetOutputCubemap"));
            GetBackgroundTexture = (ArCoreGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetBackgroundTexture"));

            if (Core::PluginManager::HasPlugin("Light Estimation LUT"))
            {
                m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
                m_pSkyComponent->SetTexture(GetOutputCubemap());
                m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
                m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX128);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);
            }

            if (Core::PluginManager::HasPlugin("Light Estimation LUT") && Core::PluginManager::HasPlugin("ArCore"))
            {
                 SetInputTexture(GetBackgroundTexture());
            }
            else
            {
                Gfx::STextureDescriptor TextureDescriptor;

                TextureDescriptor.m_NumberOfPixelsU  = 512;
                TextureDescriptor.m_NumberOfPixelsV  = 512;
                TextureDescriptor.m_NumberOfPixelsW  = 1;
                TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
                TextureDescriptor.m_NumberOfTextures = 1;
                TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
                TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
                TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
                TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
                TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
                TextureDescriptor.m_pFileName        = "../../plugins/light_estimation_lut/face_x.png";
                TextureDescriptor.m_pPixels          = 0;
                TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;

                Gfx::CTexturePtr m_InputTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                Gfx::TextureManager::SetTextureLabel(m_InputTexturePtr, "INPUT");

                SetInputTexture(m_InputTexturePtr);
            }
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }
    };
} // namespace Scpt