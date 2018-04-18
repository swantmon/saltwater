
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

            if (Core::PluginManager::HasPlugin("Light Estimation LUT") && Core::PluginManager::HasPlugin("ArCore"))
            {
                m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
                m_pSkyComponent->SetTexture(GetOutputCubemap());
                m_pSkyComponent->SetRefreshMode(Dt::CSkyComponent::Dynamic);
                m_pSkyComponent->SetQuality(Dt::CSkyComponent::PX128);
                m_pSkyComponent->SetIntensity(12000);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);

                SetInputTexture(GetBackgroundTexture());
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