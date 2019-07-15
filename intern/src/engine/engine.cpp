
#include "engine/engine_precompiled.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/network/core_network_manager.h"
#include "engine/network/core_network_socket.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/core/core_plugin_manager.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/data/data_entity_manager.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_pipeline.h"

#include "engine/gui/gui_input_manager.h"

#include "engine/script/script_script_manager.h"

#include <array>

using namespace Engine;

namespace 
{
    class CEngine : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CEngine)

    public:

        CEngine();

        ~CEngine();

        void Startup();

        void Shutdown();

        void Update();

        void Resume();

        void Pause();

        CEventDelegates::HandleType RegisterEventHandler(EEvent _Event, CEventDelegates::FunctionType _Function);

        void RaiseEvent(EEvent _Event);

    private:

        CEventDelegates m_OnEventDelegates;
    };
} // namespace 

namespace 
{
    CEngine::CEngine()
    {

    }

    // -----------------------------------------------------------------------------

    CEngine::~CEngine()
    {
    }

    // -----------------------------------------------------------------------------

    void CEngine::Startup()
    {
        Core::Time::OnStart();

        Scpt::ScriptManager::OnStart();

        Dt::CEntityManager::GetInstance().OnStart();

        Gui::InputManager::OnStart();

        Net::CNetworkManager::GetInstance().OnStart();

        Gfx::Pipeline::OnStart();

        Core::PluginManager::OnStart();

        RaiseEvent(EEvent::Engine_OnStartup);
    }

    // -----------------------------------------------------------------------------

    void CEngine::Shutdown()
    {
        RaiseEvent(EEvent::Engine_OnShutdown);

        Core::PluginManager::OnExit();

        Scpt::ScriptManager::OnExit();

        Dt::CEntityManager::GetInstance().OnExit();

        Gui::InputManager::OnExit();

        Net::CNetworkManager::GetInstance().OnExit();

        Gfx::Pipeline::OnExit();

        Core::Time::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CEngine::Update()
    {
        Core::PluginManager::Update();

        Core::Time::Update();

        Cam::ControlManager::Update();

        Dt::CEntityManager::GetInstance().Update();

        Scpt::ScriptManager::Update();

        Cam::ControlManager::Update();

        Gui::InputManager::Update();

        Net::CNetworkManager::GetInstance().Update();

        Gfx::Pipeline::Render();

        RaiseEvent(EEvent::Engine_OnUpdate);
    }

    // -----------------------------------------------------------------------------

    void CEngine::Resume()
    {
        Core::PluginManager::OnResume();

        RaiseEvent(EEvent::Engine_OnResume);
    }

    // -----------------------------------------------------------------------------

    void CEngine::Pause()
    {
        Core::PluginManager::OnPause();

        RaiseEvent(EEvent::Engine_OnPause);
    }

    // -----------------------------------------------------------------------------

    CEventDelegates::HandleType CEngine::RegisterEventHandler(EEvent _Event, CEventDelegates::FunctionType _Function)
    {
        return m_OnEventDelegates.Register(static_cast<int>(_Event), _Function);
    }

    // -----------------------------------------------------------------------------

    void CEngine::RaiseEvent(EEvent _Event)
    {
        m_OnEventDelegates.Notify(static_cast<int>(_Event));
    }

} // namespace 

namespace Engine
{
    void Startup()
    {
        CEngine::GetInstance().Startup();
    }

    // -----------------------------------------------------------------------------

    void Shutdown()
    {
        CEngine::GetInstance().Shutdown();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CEngine::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Resume()
    {
        CEngine::GetInstance().Resume();
    }

    // -----------------------------------------------------------------------------

    void Pause()
    {
        CEngine::GetInstance().Pause();
    }

    // -----------------------------------------------------------------------------

    CEventDelegates::HandleType RegisterEventHandler(EEvent _Event, CEventDelegates::FunctionType _Function)
    {
        return CEngine::GetInstance().RegisterEventHandler(_Event, _Function);
    }

    // -----------------------------------------------------------------------------

    void RaiseEvent(EEvent _Event)
    {
        CEngine::GetInstance().RaiseEvent(_Event);
    }

} // namespace Pipeline