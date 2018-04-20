
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CEasyARCameraControlScript : public CScript<CEasyARCameraControlScript>
    {
    public:

        typedef Gfx::CTexturePtr (*ArGetBackgroundTextureFunc)();

        ArGetBackgroundTextureFunc GetBackgroundTexture;

    public:

        Dt::CEntity* m_pCameraEntity = nullptr;
        Dt::CCameraComponent* m_pCameraComponent = nullptr;
        bool m_ArAvailable = false;

    private:

        glm::mat3 m_MRToEngineMatrix = glm::mat3(1.0f);

    public:

        void Start() override
        {
            m_pCameraEntity = GetEntity();

            if (m_pCameraEntity != nullptr)
            {
                m_pCameraComponent = m_pCameraEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>();
            }

            m_MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));

            m_ArAvailable = Core::PluginManager::HasPlugin("EasyAR");

            GetBackgroundTexture = (ArGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "GetBackgroundTexture"));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (!m_ArAvailable) return;

            if (m_pCameraComponent != nullptr)
            {
                m_pCameraComponent->SetClearFlag(Dt::CCameraComponent::Texture);

                m_pCameraComponent->SetProjectionType(Dt::CCameraComponent::External);

                m_pCameraComponent->SetBackgroundTexture(GetBackgroundTexture());

                m_pCameraComponent->SetFlipVertical(true);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pCameraComponent, Dt::CCameraComponent::DirtyInfo);
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }
    };
} // namespace Scpt