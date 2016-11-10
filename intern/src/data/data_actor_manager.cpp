
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::ActorManager;

namespace
{
    class CDtActorManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtActorManager);
        
    public:
        
        CDtActorManager();
        ~CDtActorManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CMeshActorFacet* CreateModelActor();
        CARActorFacet* CreateARActor();
        CCameraActorFacet* CreateCameraActor();

        void Update();
        
    private:
        
        class CInternModelActorFacet : public CMeshActorFacet
        {
        private:
            friend class CDtActorManager;
        };

        class CInternARActorFacet : public CARActorFacet
        {
        private:
            friend class CDtActorManager;
        };

        class CInternCameraActorFacet : public CCameraActorFacet
        {
        private:
            friend class CDtActorManager;
        };
        
    private:
        
        typedef Base::CPool<CInternModelActorFacet, 2048> CModelActorFacetPool;
        typedef Base::CPool<CInternARActorFacet, 8> CARActorFacetPool;
        typedef Base::CPool<CInternCameraActorFacet, 8> CCameraActorFacetPool;

    private:
        
        CModelActorFacetPool  m_ModelActorFacets;
        CARActorFacetPool     m_ARActorFacets;
        CCameraActorFacetPool m_CameraActorFacets;
    };
} // namespace

namespace
{
    CDtActorManager::CDtActorManager()
        : m_ModelActorFacets ()
        , m_ARActorFacets    ()
        , m_CameraActorFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtActorManager::~CDtActorManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtActorManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtActorManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtActorManager::Clear()
    {
        m_ModelActorFacets .Clear();
        m_ARActorFacets    .Clear();
        m_CameraActorFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet* CDtActorManager::CreateModelActor()
    {
        CInternModelActorFacet& rDataModelActorFacet = m_ModelActorFacets.Allocate();

        return &rDataModelActorFacet;
    }

    // -----------------------------------------------------------------------------

    CARActorFacet* CDtActorManager::CreateARActor()
    {
        CInternARActorFacet& rDataARActorFacet = m_ARActorFacets.Allocate();

        return &rDataARActorFacet;
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet* CDtActorManager::CreateCameraActor()
    {
        CInternCameraActorFacet& rDataCameraActorFacet = m_CameraActorFacets.Allocate();

        return &rDataCameraActorFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtActorManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace ActorManager
{
    void OnStart()
    {
        CDtActorManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtActorManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtActorManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet* CreateModelActor()
    {
        return CDtActorManager::GetInstance().CreateModelActor();
    }

    // -----------------------------------------------------------------------------

    CARActorFacet* CreateARActor()
    {
        return CDtActorManager::GetInstance().CreateARActor();
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet* CreateCameraActor()
    {
        return CDtActorManager::GetInstance().CreateCameraActor();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtActorManager::GetInstance().Update();
    }
} // namespace ActorManager
} // namespace Dt
