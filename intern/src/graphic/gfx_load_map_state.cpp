
#include "graphic/gfx_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_load_map_state.h"

namespace
{
    class CGfxLoadMapState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLoadMapState)
        
    public:
        
        void OnEnter(Base::CTextReader& _rSerializer);
        void OnLeave();
        void OnRun();
    };
} // namespace

namespace
{
    void CGfxLoadMapState::OnEnter(Base::CTextReader& _rSerializer)
    {
        BASE_UNUSED(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLoadMapState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLoadMapState::OnRun()
    {
        
    }
} // namespace

namespace Gfx
{
namespace LoadMap
{
    void OnEnter(Base::CTextReader& _rSerializer)
    {
        CGfxLoadMapState::GetInstance().OnEnter(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxLoadMapState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxLoadMapState::GetInstance().OnRun();
    }
} // namespace LoadMap
} // namespace Gfx