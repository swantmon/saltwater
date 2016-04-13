
#include "base/base_defines.h"

#include "graphic/gfx_unload_map_state.h"

namespace
{
    class CGfxUnloadMapState
    {
        
    public:
        
        void OnEnter(Base::CTextWriter& _rSerializer);
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    CGfxUnloadMapState g_GfxUnloadMapState;
} // namespace

namespace
{
    void CGfxUnloadMapState::OnEnter(Base::CTextWriter& _rSerializer)
    {
        BASE_UNUSED(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxUnloadMapState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxUnloadMapState::OnRun()
    {
        
    }
} // namespace

namespace Gfx
{
namespace UnloadMap
{
    void OnEnter(Base::CTextWriter& _rSerializer)
    {
        g_GfxUnloadMapState.OnEnter(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        g_GfxUnloadMapState.OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        g_GfxUnloadMapState.OnRun();
    }
} // namespace UnloadMap
} // namespace Gfx