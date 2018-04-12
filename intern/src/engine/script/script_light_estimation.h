
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_texture.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CLightEstimationScript : public CScript<CLightEstimationScript>
    {
    public:

        typedef void (*LightEstimationLUTSetInputTextureFunc)(Gfx::CTexturePtr);
        typedef Gfx::CTexturePtr (*LightEstimationLUTGetOutputCubemapFunc)();

        LightEstimationLUTSetInputTextureFunc SetInputTexture;
        LightEstimationLUTGetOutputCubemapFunc GetOutputCubemap;

    public:

        Dt::CEntity* m_pSkyEntity = nullptr;
        Dt::CSkyComponent* m_pSkyComponent = nullptr;
        bool m_PluginAvailable = false;

    public:

        void Start() override
        {
            m_pSkyEntity = GetEntity();

            if (m_pSkyEntity != nullptr)
            {
                m_pSkyComponent = m_pSkyEntity->GetComponentFacet()->GetComponent<Dt::CSkyComponent>();
            }

            if (m_pSkyComponent == nullptr) return;

            m_PluginAvailable = Core::PluginManager::HasPlugin("Light Estimation LUT");

            SetInputTexture  = (LightEstimationLUTSetInputTextureFunc)(Core::PluginManager::GetPluginFunction("Light Estimation LUT", "SetInputTexture"));
            GetOutputCubemap = (LightEstimationLUTGetOutputCubemapFunc)(Core::PluginManager::GetPluginFunction("Light Estimation LUT", "GetOutputCubemap"));

            if (m_PluginAvailable)
            {
                m_pSkyComponent->SetType(Dt::CSkyComponent::Cubemap);
                m_pSkyComponent->SetTexture(GetOutputCubemap());

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pSkyComponent, Dt::CSkyComponent::DirtyInfo);
            }
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (!m_PluginAvailable) return;
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }
    };
} // namespace Scpt