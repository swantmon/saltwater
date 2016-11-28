
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_mesh_facet.h"
#include "data/data_mesh_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::MeshManager;

namespace
{
    class CDtMeshManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtMeshManager);
        
    public:
        
        CDtMeshManager();
        ~CDtMeshManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CMeshActorFacet* CreateModelActor();

        void Update();
        
    private:
        
        class CInternModelActorFacet : public CMeshActorFacet
        {
        private:
            friend class CDtMeshManager;
        };
        
    private:
        
        typedef Base::CPool<CInternModelActorFacet, 2048> CModelActorFacetPool;

    private:
        
        CModelActorFacetPool m_ModelActorFacets;
    };
} // namespace

namespace
{
    CDtMeshManager::CDtMeshManager()
        : m_ModelActorFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMeshManager::~CDtMeshManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::Clear()
    {
        m_ModelActorFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet* CDtMeshManager::CreateModelActor()
    {
        CInternModelActorFacet& rDataModelActorFacet = m_ModelActorFacets.Allocate();

        return &rDataModelActorFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace MeshManager
{
    void OnStart()
    {
        CDtMeshManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtMeshManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtMeshManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet* CreateModelActor()
    {
        return CDtMeshManager::GetInstance().CreateModelActor();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtMeshManager::GetInstance().Update();
    }
} // namespace MeshManager
} // namespace Dt
