
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_intro_state.h"

namespace
{
    class CGfxIntroState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxIntroState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGfxIntroState::OnEnter()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxIntroState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxIntroState::OnRun()
    {
        
    }
} // namespace

namespace Gfx
{
namespace Intro
{
    void OnEnter()
    {
        CGfxIntroState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxIntroState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxIntroState::GetInstance().OnRun();
    }
} // namespace Intro
} // namespace Gfx