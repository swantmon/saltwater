
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_main_menu_state.h"

namespace
{
    class CGfxMainMenuState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxMainMenuState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGfxMainMenuState::OnEnter()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMainMenuState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMainMenuState::OnRun()
    {
        
    }
} // namespace

namespace Gfx
{
namespace MainMenu
{
    void OnEnter()
    {
        CGfxMainMenuState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxMainMenuState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxMainMenuState::GetInstance().OnRun();
    }
} // namespace MainMenu
} // namespace Gfx
