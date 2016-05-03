
#include "gui/gui_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_debug_state.h"

namespace
{
    class CGuiDebugState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiDebugState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace
namespace
{
    void CGuiDebugState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiDebugState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiDebugState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace Debug
{
    void OnEnter()
    {
        CGuiDebugState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiDebugState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiDebugState::GetInstance().OnRun();
    }
} // namespace Debug
} // namespace Gui