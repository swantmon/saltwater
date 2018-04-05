
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_main_menu_state.h"

namespace
{
    class CGuiMainMenuState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiMainMenuState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiMainMenuState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiMainMenuState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiMainMenuState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace MainMenu
{
    void OnEnter()
    {
        CGuiMainMenuState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiMainMenuState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiMainMenuState::GetInstance().OnRun();
    }
} // namespace MainMenu
} // namespace Gui
