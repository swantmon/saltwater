
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_camera_facet.h"
#include "data/data_camera_manager.h"

using namespace Dt;
using namespace Dt::CameraManager;

namespace
{
    class CDtCameraManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtCameraManager);
        
    public:
        
        CDtCameraManager();
        ~CDtCameraManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CCameraActorFacet* CreateCameraActor();

        void Update();
        
    private:
        
        class CInternCameraActorFacet : public CCameraActorFacet
        {
        private:
            friend class CDtCameraManager;
        };
        
    private:

        typedef Base::CPool<CInternCameraActorFacet, 8> CCameraActorFacetPool;

    private:
        
        CCameraActorFacetPool m_CameraActorFacets;
    };
} // namespace

namespace
{
    CDtCameraManager::CDtCameraManager()
        : m_CameraActorFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtCameraManager::~CDtCameraManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtCameraManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtCameraManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtCameraManager::Clear()
    {
        m_CameraActorFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet* CDtCameraManager::CreateCameraActor()
    {
        CInternCameraActorFacet& rDataCameraActorFacet = m_CameraActorFacets.Allocate();

        return &rDataCameraActorFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtCameraManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace CameraManager
{
    void OnStart()
    {
        CDtCameraManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtCameraManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtCameraManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet* CreateCameraActor()
    {
        return CDtCameraManager::GetInstance().CreateCameraActor();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtCameraManager::GetInstance().Update();
    }
} // namespace CameraManager
} // namespace Dt
