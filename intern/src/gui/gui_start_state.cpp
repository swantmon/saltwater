
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_start_state.h"

namespace
{
    class CGuiStartState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiStartState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiStartState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiStartState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiStartState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace Start
{
    void OnEnter()
    {
        CGuiStartState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiStartState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiStartState::GetInstance().OnRun();
    }
} // namespace Start
} // namespace Gui