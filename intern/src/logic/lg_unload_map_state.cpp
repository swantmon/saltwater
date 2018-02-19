
#include "logic/lg_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity_manager.h"
#include "data/data_map.h"

#include "logic/lg_unload_map_state.h"

namespace
{
    class CLgUnloadMapState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgUnloadMapState)
        
    public:
        
        int OnEnter(Base::CTextWriter& _rSerializer);
        int OnLeave();
        int OnRun();
        
    };
} // namespace

namespace
{
    int CLgUnloadMapState::OnEnter(Base::CTextWriter& _rSerializer)
    {
        BASE_UNUSED(_rSerializer);

        // -----------------------------------------------------------------------------
        // Notify map that level should be unloaded
        // -----------------------------------------------------------------------------
        Dt::Map::FreeMap();

        // -----------------------------------------------------------------------------
        // Clear entities
        // -----------------------------------------------------------------------------
        Dt::EntityManager::Clear();
        
        return Lg::UnloadMap::SResult::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgUnloadMapState::OnLeave()
    {
        return Lg::UnloadMap::SResult::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgUnloadMapState::OnRun()
    {
        return Lg::UnloadMap::SResult::Exit;
    }
} // namespace

namespace Lg
{
namespace UnloadMap
{
    int OnEnter(Base::CTextWriter& _rSerializer)
    {
        return CLgUnloadMapState::GetInstance().OnEnter(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgUnloadMapState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgUnloadMapState::GetInstance().OnRun();
    }
} // namespace UnloadMap
} // namespace Lg