
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_ar_controller_facet.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_plugin_type.h"
#include "data/data_transformation_facet.h"

#include "mr/mr_control_manager.h"

#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include <assert.h>
#include <unordered_map>
#include <vector>

using namespace MR;

namespace
{
    class CMRControlManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRControlManager);

    public:

        static const unsigned int s_NumberOfFramesUntilCameraIsInvisible = 0;
        
    public:

        CMRControlManager();
       ~CMRControlManager();

    public:

        void OnStart();
        void OnExit();
        void Update();

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (_pEntity->GetCategory() != Dt::SEntityCategory::Plugin) return;

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // ...
        }
    }
} // namespace

namespace MR
{
namespace ControlManager
{
    void OnStart()
    {
        CMRControlManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRControlManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRControlManager::GetInstance().Update();
    }
} // namespace ControlManager
} // namespace MR
