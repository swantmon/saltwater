
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

#include <map>

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

        void RegisterEventHandler(int _EventID, CEventDelegate _Delegate);

        void RaiseEvent(int _EventID);

    private:

        typedef std::map<int, std::vector<CEventDelegate>> CEventDelegates;

    private:

        CEventDelegates m_EventDelegates;
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
        m_EventDelegates.clear();
    }

    // -----------------------------------------------------------------------------

    void CEngine::Startup()
    {
        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
        Core::Time::OnStart();

        Scpt::ScriptManager::OnStart();

        Dt::EntityManager::OnStart();

        Gui::InputManager::OnStart();

        Net::CNetworkManager::GetInstance().OnStart();

        Gfx::Pipeline::OnStart();

		Core::PluginManager::Start();
    }

    // -----------------------------------------------------------------------------

    void CEngine::Shutdown()
    {
        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
		Core::PluginManager::Exit();
		
		Scpt::ScriptManager::OnExit();

        Dt::EntityManager::OnExit();

        Gui::InputManager::OnExit();

        Net::CNetworkManager::GetInstance().OnExit();

        Gfx::Pipeline::OnExit();

        Core::Time::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CEngine::Update()
    {
        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
		Core::PluginManager::Update();

        Core::Time::Update();

        Cam::ControlManager::Update();

        Dt::EntityManager::Update();

        Scpt::ScriptManager::Update();

        Cam::ControlManager::Update();

        Gui::InputManager::Update();

        Net::CNetworkManager::GetInstance().Update();

        Gfx::Pipeline::Render();
    }

    // -----------------------------------------------------------------------------

    void CEngine::Resume()
    {
        // -----------------------------------------------------------------------------
        // Plugins
        // -----------------------------------------------------------------------------
		Core::PluginManager::Resume();
    }

    // -----------------------------------------------------------------------------

    void CEngine::Pause()
    {
        // -----------------------------------------------------------------------------
        // Plugins
        // -----------------------------------------------------------------------------
		Core::PluginManager::Pause();
    }

    // -----------------------------------------------------------------------------

    void CEngine::RegisterEventHandler(int _EventID, CEventDelegate _Delegate)
    {
        m_EventDelegates[_EventID].push_back(_Delegate);
    }

    // -----------------------------------------------------------------------------

    void CEngine::RaiseEvent(int _EventID)
    {
        auto& rListOfDelegates = m_EventDelegates[_EventID];

        for (auto& rEvent : rListOfDelegates)
        {
            rEvent();
        }
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

    void RegisterEventHandler(int _EventID, CEventDelegate _Delegate)
    {
        CEngine::GetInstance().RegisterEventHandler(_EventID, _Delegate);
    }

    // -----------------------------------------------------------------------------

    void RaiseEvent(int _EventID)
    {
        CEngine::GetInstance().RaiseEvent(_EventID);
    }

} // namespace Pipeline