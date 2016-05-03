
#include "gui/gui_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_play_state.h"

namespace
{
    class CGuiPlayState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiPlayState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiPlayState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiPlayState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiPlayState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace Play
{
    void OnEnter()
    {
        CGuiPlayState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiPlayState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiPlayState::GetInstance().OnRun();
    }
} // namespace Play
} // namespace Gui