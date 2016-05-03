
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_Plugin_facet.h"
#include "data/data_Plugin_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::PluginManager;

namespace
{
    class CDtPluginManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtPluginManager);
        
    public:
        
        CDtPluginManager();
        ~CDtPluginManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CARControllerPluginFacet* CreateARControllerPlugin();
        CARTrackedObjectPluginFacet* CreateARTrackedObjectPlugin();

        void Update();
        
    private:
        
        class CInternARControllerPluginFacet : public CARControllerPluginFacet
        {
        private:
            friend class CDtPluginManager;
        };

        class CInternARTrackedObjectPluginFacet : public CARTrackedObjectPluginFacet
        {
        private:
            friend class CDtPluginManager;
        };
        
    private:
        
        typedef Base::CPool<CInternARControllerPluginFacet, 1> CARControllerPluginFacetPool;
        typedef Base::CPool<CInternARTrackedObjectPluginFacet, 8> CARTrackedObjectPluginFacetPool;

    private:
        
        CARControllerPluginFacetPool    m_ARControllerPluginFacets;
        CARTrackedObjectPluginFacetPool m_ARTrackedObjectPluginFacets;
    };
} // namespace

namespace
{
    CDtPluginManager::CDtPluginManager()
        : m_ARControllerPluginFacets   ()
        , m_ARTrackedObjectPluginFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtPluginManager::~CDtPluginManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtPluginManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtPluginManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtPluginManager::Clear()
    {
        m_ARControllerPluginFacets .Clear();
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet* CDtPluginManager::CreateARControllerPlugin()
    {
        CInternARControllerPluginFacet& rDataPluginFacet = m_ARControllerPluginFacets.Allocate();

        return &rDataPluginFacet;
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginFacet* CDtPluginManager::CreateARTrackedObjectPlugin()
    {
        CInternARTrackedObjectPluginFacet& rDataPluginFacet = m_ARTrackedObjectPluginFacets.Allocate();

        return &rDataPluginFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtPluginManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace PluginManager
{
    void OnStart()
    {
        CDtPluginManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtPluginManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtPluginManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet* CreateARControllerPlugin()
    {
        return CDtPluginManager::GetInstance().CreateARControllerPlugin();
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginFacet* CreateARTrackedObjectPlugin()
    {
        return CDtPluginManager::GetInstance().CreateARTrackedObjectPlugin();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtPluginManager::GetInstance().Update();
    }
} // namespace PluginManager
} // namespace Dt
