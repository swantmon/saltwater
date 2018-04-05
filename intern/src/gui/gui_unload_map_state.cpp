
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_unload_map_state.h"

namespace
{
    class CGuiUnloadMapState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiUnloadMapState)
        
    public:
        
        void OnEnter(Base::CTextWriter& _rSerializer);
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGuiUnloadMapState::OnEnter(Base::CTextWriter& _rSerializer)
    {
        BASE_UNUSED(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiUnloadMapState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiUnloadMapState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace UnloadMap
{
    void OnEnter(Base::CTextWriter& _rSerializer)
    {
        CGuiUnloadMapState::GetInstance().OnEnter(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiUnloadMapState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiUnloadMapState::GetInstance().OnRun();
    }
} // namespace UnloadMap
} // namespace Gui