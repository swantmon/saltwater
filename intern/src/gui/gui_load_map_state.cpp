
#include "gui/gui_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_load_map_state.h"

namespace
{
    class CGuiLoadMapState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiLoadMapState)
        
    public:
        
        void OnEnter(Base::CTextReader& _rSerializer);
        void OnLeave();
        void OnRun();
    };
} // namespace

namespace
{
    void CGuiLoadMapState::OnEnter(Base::CTextReader& _rSerializer)
    {
        BASE_UNUSED(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiLoadMapState::OnLeave()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGuiLoadMapState::OnRun()
    {
    }
} // namespace

namespace Gui
{
namespace LoadMap
{
    void OnEnter(Base::CTextReader& _rSerializer)
    {
        CGuiLoadMapState::GetInstance().OnEnter(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGuiLoadMapState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGuiLoadMapState::GetInstance().OnRun();
    }
} // namespace LoadMap
} // namespace Gui