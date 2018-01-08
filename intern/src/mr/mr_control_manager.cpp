
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

#include "arcore_c_api.h"

#include <assert.h>
#include <unordered_map>
#include <vector>

using namespace MR;
using namespace MR::ControlManager;

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

        void OnStart(const SConfiguration& _rConfiguration);
        void OnExit();
        void Update();

    private:

        ArSession* ar_session_ = nullptr;
        ArFrame* ar_frame_ = nullptr;

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

    void CMRControlManager::OnStart(const SConfiguration& _rConfiguration)
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        ArStatus Status;

        Status = ArSession_create(_rConfiguration.m_pEnv, _rConfiguration.m_pContext, &ar_session_);

        if (Status == AR_SUCCESS)
        {
            ArConfig* ar_config = nullptr;
            ArConfig_create(ar_session_, &ar_config);
        }
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
    void OnStart(const SConfiguration& _rConfiguration)
    {
        CMRControlManager::GetInstance().OnStart(_rConfiguration);
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
