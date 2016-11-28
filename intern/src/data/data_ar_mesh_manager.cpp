
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ar_mesh_facet.h"
#include "data/data_ar_mesh_manager.h"

using namespace Dt;
using namespace Dt::ARMeshManager;

namespace
{
    class CDtARMeshManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtARMeshManager);
        
    public:
        
        CDtARMeshManager();
        ~CDtARMeshManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CARActorFacet* CreateARActor();

        void Update();
        
    private:
        
        class CInternARActorFacet : public CARActorFacet
        {
        private:
            friend class CDtARMeshManager;
        };
        
    private:
        
        typedef Base::CPool<CInternARActorFacet, 8> CARActorFacetPool;

    private:
        
        CARActorFacetPool     m_ARActorFacets;
    };
} // namespace

namespace
{
    CDtARMeshManager::CDtARMeshManager()
        : m_ARActorFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtARMeshManager::~CDtARMeshManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtARMeshManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtARMeshManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtARMeshManager::Clear()
    {
        m_ARActorFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CARActorFacet* CDtARMeshManager::CreateARActor()
    {
        CInternARActorFacet& rDataARActorFacet = m_ARActorFacets.Allocate();

        return &rDataARActorFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtARMeshManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace ARMeshManager
{
    void OnStart()
    {
        CDtARMeshManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtARMeshManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtARMeshManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CARActorFacet* CreateARActor()
    {
        return CDtARMeshManager::GetInstance().CreateARActor();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtARMeshManager::GetInstance().Update();
    }
} // namespace ARMeshManager
} // namespace Dt
