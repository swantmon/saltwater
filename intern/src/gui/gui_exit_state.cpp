
#include "gui/gui_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_exit_state.h"

namespace
{
    class CGuiExitState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiExitState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiExitState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiExitState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiExitState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace Exit
{
    void OnEnter()
    {
        CGuiExitState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiExitState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiExitState::GetInstance().OnRun();
    }
} // namespace Exit
} // namespace Gui