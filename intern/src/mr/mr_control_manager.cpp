
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

        CMRControlManager();
       ~CMRControlManager();

    public:

        void OnStart(const SConfiguration& _rConfiguration);
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

    private:

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
        : m_pARSession(0)
        , m_pARFrame  (0)
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

        Status = ArSession_create(_rConfiguration.m_pEnv, _rConfiguration.m_pContext, &m_pARSession);

        assert(Status == AR_SUCCESS);

        assert(m_pARSession != 0);

        ArConfig* ARConfig = 0;

        ArConfig_create(m_pARSession, &ARConfig);

        assert(ARConfig != 0);

        Status = ArSession_checkSupported(m_pARSession, ARConfig);

        assert(Status == AR_SUCCESS);

        Status = ArSession_configure(m_pARSession, ARConfig);

        assert(Status == AR_SUCCESS);

        ArConfig_destroy(ARConfig);

        ArFrame_create(m_pARSession, &m_pARFrame);

        assert(m_pARFrame != 0);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
        ArSession_destroy(m_pARSession);

        ArFrame_destroy(m_pARFrame);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        if (ArSession_update(m_pARSession, m_pARFrame) != AR_SUCCESS)
        {
            BASE_CONSOLE_ERROR("HelloArApplication::OnDrawFrame ArSession_update error");
        }

        // -----------------------------------------------------------------------------
        // Update camera
        // -----------------------------------------------------------------------------
        ArCamera* pARCamera;

        ArFrame_acquireCamera(m_pARSession, m_pARFrame, &pARCamera);

        Base::Float4x4 ViewMatrix;
        Base::Float4x4 ProjectionMatrix;

        ArCamera_getViewMatrix(m_pARSession, pARCamera, &ViewMatrix[0][0]);

        ArCamera_getProjectionMatrix(m_pARSession, pARCamera, 0.1f, 100.0f, &ProjectionMatrix[0][0]);

        ArCamera_release(pARCamera);

        // TODO: send view and proj. matrix to camera

        // -----------------------------------------------------------------------------
        // Light estimation
        // Intensity value ranges from 0.0f to 1.0f.
        // -----------------------------------------------------------------------------
        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(m_pARSession, &ARLightEstimate);

        ArFrame_getLightEstimate(m_pARSession, m_pARFrame, ARLightEstimate);

        ArLightEstimate_getState(m_pARSession, ARLightEstimate, &ARLightEstimateState);

        float LightIntensity = 0.8f;

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getPixelIntensity(m_pARSession, ARLightEstimate, &LightIntensity);
        }

        ArLightEstimate_destroy(ARLightEstimate);

        ARLightEstimate = nullptr;

        // TODO: use light estimation for our lighting

        // -----------------------------------------------------------------------------
        // Find trackables
        // -----------------------------------------------------------------------------
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnPause()
    {
        ArSession_pause(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResume()
    {
        ArStatus Status = ArSession_resume(m_pARSession);

        assert(Status == AR_SUCCESS);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        ArSession_setDisplayGeometry(m_pARSession, _DisplayRotation, _Width, _Height);
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

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CMRControlManager::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CMRControlManager::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        CMRControlManager::GetInstance().OnDisplayGeometryChanged(_DisplayRotation, _Width, _Height);
    }
} // namespace ControlManager
} // namespace MR
