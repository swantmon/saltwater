
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_transformation_facet.h"

#include "mr/mr_camera_manager.h"
#include "mr/mr_control_manager.h"
#include "mr/mr_light_estimation_manager.h"
#include "mr/mr_marker_manager.h"
#include "mr/mr_session_manager.h"

#include "script/script_script.h"

namespace Scpt
{
    class CARControllerScript : public CScript<CARControllerScript>
    {
    public:

        void Start() override
        {
            MR::ControlManager::OnStart();
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            MR::ControlManager::OnExit();
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            MR::ControlManager::Update();
            MR::SessionManager::Update();
            MR::CameraManager::Update();
            MR::MarkerManager::Update();
            MR::LightEstimationManager::Update();
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
        }
    };
} // namespace Scpt