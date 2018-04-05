
#include "engine/engine_precompiled.h"
#include "engine/engine.h"

#include "camera/cam_control_manager.h"

#include "core/core_time.h"

#include "data/data_entity_manager.h"

#include "graphic/gfx_engine.h"

#include "gui/gui_input_manager.h"

#include "script/script_script_manager.h"

namespace Engine
{
    void Startup()
    {
        Core::Time::OnStart();

        Scpt::ScriptManager::OnStart();

        Dt::EntityManager::OnStart();

        Gui::InputManager::OnStart();

        Gfx::Engine::OnStart();
    }

    // -----------------------------------------------------------------------------

    void Exit()
    {
        Scpt::ScriptManager::OnExit();

        Dt::EntityManager::OnExit();

        Gui::InputManager::OnExit();

        Gfx::Engine::OnExit();

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

        Gfx::Engine::Render();
    }
} // namespace Engine