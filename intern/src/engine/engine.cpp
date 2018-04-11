
#include "engine/engine_precompiled.h"
#include "engine/engine.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/core/core_time.h"

#include "engine/data/data_entity_manager.h"

#include "engine/graphic/gfx_pipeline.h"

#include "engine/gui/gui_input_manager.h"

#include "engine/script/script_script_manager.h"

namespace Engine
{
    void Startup()
    {
        Core::Time::OnStart();

        Scpt::ScriptManager::OnStart();

        Dt::EntityManager::OnStart();

        Gui::InputManager::OnStart();

        Gfx::Pipeline::OnStart();
    }

    // -----------------------------------------------------------------------------

    void Shutdown()
    {
        Scpt::ScriptManager::OnExit();

        Dt::EntityManager::OnExit();

        Gui::InputManager::OnExit();

        Gfx::Pipeline::OnExit();

        Core::Time::OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        Core::Time::Update();

        Cam::ControlManager::Update();

        Dt::EntityManager::Update();

        Scpt::ScriptManager::Update();

        Cam::ControlManager::Update();

        Gui::InputManager::Update();

        Gfx::Pipeline::Render();
    }
} // namespace Pipeline