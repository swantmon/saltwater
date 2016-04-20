
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_edit_state.h"

namespace
{
    class CGuiEditState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiEditState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiEditState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiEditState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiEditState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace Edit
{
    void OnEnter()
    {
        CGuiEditState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiEditState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiEditState::GetInstance().OnRun();
    }
} // namespace Edit
} // namespace Gui