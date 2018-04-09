
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "core/core_plugin_manager.h"

#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_transformation_facet.h"

#include "script/script_script.h"

namespace Scpt
{
    class CARControllerScript : public CScript<CARControllerScript>
    {
    public:

        typedef const void* (*ArCoreGetCameraFunc)();
        typedef int (*ArCoreGetCameraTrackingStateFunc)(const void* _pCamera);

    public:

        ArCoreGetCameraFunc ArCoreGetCamera;
        ArCoreGetCameraTrackingStateFunc ArCoreGetCameraTrackingState;

    public:

        void Start() override
        {
            ArCoreGetCamera = (ArCoreGetCameraFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetCamera"));
            ArCoreGetCameraTrackingState = (ArCoreGetCameraTrackingStateFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetCameraTrackingState"));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (   ArCoreGetCamera              == nullptr
                || ArCoreGetCameraTrackingState == nullptr)
            {
                return;
            }

            const void* rCamera = ArCoreGetCamera();

            int State = ArCoreGetCameraTrackingState(rCamera);

            BASE_CONSOLE_INFOV("Tracking state is %i", State);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }
    };
} // namespace Scpt