
#include "script/script_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_script_component.h"
#include "data/data_component_manager.h"

#include "gui/gui_event_handler.h"

#include "script/script_script_manager.h"

#include <vector>

namespace 
{
    class CScptScriptManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CScptScriptManager)

    public:

        void OnStart();
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

    private:

        CScptScriptManager();
        ~CScptScriptManager();

    private:

        typedef std::vector<Dt::CScriptComponent*> CScripts;

    private:

        CScripts m_Scripts;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
    };
} // namespace 

namespace
{
    CScptScriptManager::CScptScriptManager()
    {
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CScptScriptManager::OnDirtyComponent));
    }

    // -----------------------------------------------------------------------------

    CScptScriptManager::~CScptScriptManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::OnStart()
    {
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CScptScriptManager::OnInputEvent));
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::OnExit()
    {
        for (auto& rScript : m_Scripts)
        {
            rScript->Exit();
        }

        Gui::EventHandler::UnregisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CScptScriptManager::OnInputEvent));
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::Update()
    {
        for (auto& rScript : m_Scripts)
        {
            rScript->Update();
        }
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::OnPause()
    {
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::OnResume()
    {
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Only if component has changed
        // -----------------------------------------------------------------------------
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CScriptComponent>()) return;

        Dt::CScriptComponent* pScriptComponent = static_cast<Dt::CScriptComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pScriptComponent->GetDirtyFlags();

        if ((DirtyFlags & Dt::CScriptComponent::DirtyCreate) != 0)
        {
            if (pScriptComponent->IsActiveAndUsable())
            {
                pScriptComponent->Start();

                m_Scripts.push_back(pScriptComponent);
            }
        }

        if ((DirtyFlags & Dt::CScriptComponent::DirtyInfo) != 0)
        {
            // -----------------------------------------------------------------------------
            // Get script from list
            // -----------------------------------------------------------------------------
            auto ScriptIter = std::find_if(m_Scripts.begin(), m_Scripts.end(), [&](Dt::CScriptComponent* _pObject) { return _pObject == pScriptComponent; });

            if (!pScriptComponent->IsActiveAndUsable() && ScriptIter != m_Scripts.end())
            {
                pScriptComponent->Exit();
            }

            if (pScriptComponent->IsActiveAndUsable() && ScriptIter == m_Scripts.end())
            {
                pScriptComponent->Start();

                m_Scripts.push_back(pScriptComponent);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CScptScriptManager::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        for (auto& rScript : m_Scripts)
        {
            rScript->OnInput(_rInputEvent);
        }
    }
} // namespace

namespace Scpt
{
namespace ScriptManager
{
    void OnStart()
    {
        CScptScriptManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CScptScriptManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CScptScriptManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CScptScriptManager::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CScptScriptManager::GetInstance().OnResume();
    }
} // namespace ScriptManager
} // namespace Scpt