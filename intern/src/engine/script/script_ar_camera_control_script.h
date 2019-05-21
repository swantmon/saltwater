
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_console.h"
#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CARCameraControlScript : public CScript<CARCameraControlScript>
    {
#ifdef PLATFORM_ANDROID
        std::string PluginName = "ArCore";
        glm::mat3 MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
#else
        std::string PluginName = "EasyAR";
        glm::mat3 MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
#endif // PLATFORM_ANDROID


    public:

        using ArCoreGetCameraFunc = const void* (*)();
        using ArCoreGetCameraTrackingStateFunc = int (*)(const void* _pCamera);
        using ArCoreGetCameraViewMatrixFunc = glm::mat4 (*)(const void* _pCamera);
        using ArCoreGetCameraProjectionMatrixFunc = glm::mat4 (*)(const void* _pCamera);
        using ArCoreGetCameraNearFunc = float (*)(const void* _pCamera);
        using ArCoreGetCameraFarFunc = float (*)(const void* _pCamera);
        using ArCoreGetBackgroundTextureFunc = Gfx::CTexturePtr (*)();
        using ArSetFlipVerticalFunc = void(*)(bool _Flag);

        ArCoreGetCameraFunc ArCoreGetCamera;
        ArCoreGetCameraTrackingStateFunc ArCoreGetCameraTrackingState;
        ArCoreGetCameraViewMatrixFunc GetCameraViewMatrix;
        ArCoreGetCameraProjectionMatrixFunc GetCameraProjectionMatrix;
        ArCoreGetCameraNearFunc GetCameraNear;
        ArCoreGetCameraFarFunc GetCameraFar;
        ArCoreGetBackgroundTextureFunc GetBackgroundTexture;

    public:

        Dt::CEntity* m_pCameraEntity = nullptr;
        Dt::CCameraComponent* m_pCameraComponent = nullptr;
        bool m_ArCoreAvailable = false;

    public:

        void Start() override
        {
            m_pCameraEntity = GetEntity();

            if (m_pCameraEntity != nullptr)
            {
                m_pCameraComponent = m_pCameraEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>();
            }

            m_ArCoreAvailable = Core::PluginManager::LoadPlugin(PluginName);

            if (!m_ArCoreAvailable)
            {
                ENGINE_CONSOLE_WARNING("No plugin is loaded to enable AR.")

                return;
            }

            ArCoreGetCamera = (ArCoreGetCameraFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetCamera"));
            ArCoreGetCameraTrackingState = (ArCoreGetCameraTrackingStateFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetCameraTrackingState"));
            GetCameraViewMatrix = (ArCoreGetCameraViewMatrixFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetCameraViewMatrix"));
            GetCameraProjectionMatrix = (ArCoreGetCameraProjectionMatrixFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetCameraProjectionMatrix"));
            GetCameraNear = (ArCoreGetCameraNearFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetCameraNear"));
            GetCameraFar = (ArCoreGetCameraFarFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetCameraFar"));
            GetBackgroundTexture = (ArCoreGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction(PluginName, "GetBackgroundTexture"));

#if PLATFORM_WINDOWS
            ArSetFlipVerticalFunc SetFlipVertical;

            SetFlipVertical = (ArSetFlipVerticalFunc)(Core::PluginManager::GetPluginFunction(PluginName, "SetFlipVertical"));

            SetFlipVertical(true);
#endif
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (!m_ArCoreAvailable) return;

            const void* rCamera = ArCoreGetCamera();

            if (m_pCameraEntity != nullptr && ArCoreGetCameraTrackingState(rCamera) == 2)
            {
                glm::mat4 CameraViewMatrix = GetCameraViewMatrix(rCamera);

                glm::mat3 WSRotation = MRToEngineMatrix * glm::transpose(glm::mat3(CameraViewMatrix));

                glm::vec3 WSPosition = WSRotation * CameraViewMatrix[3] * -1.0f;

                m_pCameraEntity->SetWorldPosition(WSPosition);

                m_pCameraEntity->GetTransformationFacet()->SetPosition(WSPosition);

                m_pCameraEntity->GetTransformationFacet()->SetScale(glm::vec3(1.0f));

                m_pCameraEntity->GetTransformationFacet()->SetRotation(glm::toQuat(WSRotation));

                Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*m_pCameraEntity, Dt::CEntity::DirtyMove);
            }

            if (m_pCameraComponent != nullptr)
            {
                m_pCameraComponent->SetClearFlag(Dt::CCameraComponent::Texture);

                m_pCameraComponent->SetProjectionType(Dt::CCameraComponent::External);

                m_pCameraComponent->SetNear(GetCameraNear(rCamera));

                m_pCameraComponent->SetFar(GetCameraFar(rCamera));

                m_pCameraComponent->SetProjectionMatrix(GetCameraProjectionMatrix(rCamera));

                m_pCameraComponent->SetBackgroundTexture(GetBackgroundTexture());

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pCameraComponent, Dt::CCameraComponent::DirtyInfo);
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CARCameraControlScript();
        }
    };
} // namespace Scpt