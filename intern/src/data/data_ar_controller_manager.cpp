
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ar_controller_facet.h"
#include "data/data_ar_controller_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::ARControllerManager;

namespace
{
    class CDtARControllerManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtARControllerManager);
        
    public:
        
        CDtARControllerManager();
        ~CDtARControllerManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CARControllerPluginFacet* CreateARControllerPlugin();

        void Update();
        
    private:
        
        class CInternARControllerPluginFacet : public CARControllerPluginFacet
        {
        private:
            friend class CDtARControllerManager;
        };
        
    private:
        
        typedef Base::CPool<CInternARControllerPluginFacet, 1> CARControllerPluginFacetPool;

    private:
        
        CARControllerPluginFacetPool m_ARControllerPluginFacets;
    };
} // namespace

namespace
{
    CDtARControllerManager::CDtARControllerManager()
        : m_ARControllerPluginFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtARControllerManager::~CDtARControllerManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtARControllerManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtARControllerManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtARControllerManager::Clear()
    {
        m_ARControllerPluginFacets .Clear();
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet* CDtARControllerManager::CreateARControllerPlugin()
    {
        CInternARControllerPluginFacet& rDataPluginFacet = m_ARControllerPluginFacets.Allocate();

        return &rDataPluginFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtARControllerManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace ARControllerManager
{
    void OnStart()
    {
        CDtARControllerManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtARControllerManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtARControllerManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet* CreateARControllerPlugin()
    {
        return CDtARControllerManager::GetInstance().CreateARControllerPlugin();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtARControllerManager::GetInstance().Update();
    }
} // namespace ARControllerManager
} // namespace Dt
