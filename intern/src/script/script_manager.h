
#pragma once

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "script/script_base_script.h"

#include <memory>
#include <vector>

namespace Script
{
    class CScriptManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CScriptManager)

    public:

        CScriptManager();
        ~CScriptManager();

        void OnStart();
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        template<class TScript>
        TScript* AllocateScript(Dt::CEntity& _rEntity);

    private:

        typedef std::vector<std::unique_ptr<CBaseScript>> CScripts;

    private:

        CScripts m_Scripts;
        Base::ID m_CurrentID;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
    };
} // namespace Script

namespace Script
{
    template<class TScript>
    TScript* CScriptManager::AllocateScript(Dt::CEntity& _rEntity)
    {
        m_Scripts.emplace_back(std::unique_ptr<TScript>(new TScript()));

        TScript* pScript = static_cast<TScript*>(m_Scripts.back().get());

        pScript->m_ID        = m_CurrentID++;
        pScript->m_pEntity   = &_rEntity;
        pScript->m_IsActive  = true;
        pScript->m_IsStarted = false;

        return pScript;
    }
} // namespace Script
