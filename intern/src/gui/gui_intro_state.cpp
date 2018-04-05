
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_intro_state.h"

namespace
{
    class CGuiIntroState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiIntroState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiIntroState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiIntroState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiIntroState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace Intro
{
    void OnEnter()
    {
        CGuiIntroState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiIntroState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiIntroState::GetInstance().OnRun();
    }
} // namespace Intro
} // namespace Gui