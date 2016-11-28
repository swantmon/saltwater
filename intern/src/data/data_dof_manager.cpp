
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_dof_facet.h"
#include "data/data_dof_manager.h"

using namespace Dt;
using namespace Dt::DOFManager;

namespace
{
    class CDtDOFManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtDOFManager);
        
    public:
        
        CDtDOFManager();
        ~CDtDOFManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CDOFFXFacet* CreateDOFFX();

        void Update();
        
    private:

        class CInternDOFFXFacet : public CDOFFXFacet
        {
        private:
            friend class CDtDOFManager;
        };
        
    private:
        
        typedef Base::CPool<CInternDOFFXFacet, 2> CDOFFXFacetPool;

    private:
        
        CDOFFXFacetPool m_DOFFXFacets;
    };
} // namespace

namespace
{
    CDtDOFManager::CDtDOFManager()
        : m_DOFFXFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtDOFManager::~CDtDOFManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtDOFManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtDOFManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtDOFManager::Clear()
    {
        m_DOFFXFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CDOFFXFacet* CDtDOFManager::CreateDOFFX()
    {
        CInternDOFFXFacet& rDataDOFFXFacet = m_DOFFXFacets.Allocate();

        return &rDataDOFFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtDOFManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace DOFManager
{
    void OnStart()
    {
        CDtDOFManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtDOFManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtDOFManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CDOFFXFacet* CreateDOFFX()
    {
        return CDtDOFManager::GetInstance().CreateDOFFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtDOFManager::GetInstance().Update();
    }
} // namespace DOFManager
} // namespace Dt
