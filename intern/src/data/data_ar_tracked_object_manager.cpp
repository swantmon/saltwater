
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ar_tracked_object_facet.h"
#include "data/data_ar_tracked_object_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::ARTrackedObjectManager;

namespace
{
    class CDtARTrackedObjectManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtARTrackedObjectManager);
        
    public:
        
        CDtARTrackedObjectManager();
        ~CDtARTrackedObjectManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CARTrackedObjectPluginFacet* CreateARTrackedObjectPlugin();

        void Update();
        
    private:

        class CInternARTrackedObjectPluginFacet : public CARTrackedObjectPluginFacet
        {
        private:
            friend class CDtARTrackedObjectManager;
        };
        
    private:
        
        typedef Base::CPool<CInternARTrackedObjectPluginFacet, 1> CARTrackedObjectPluginFacetPool;

    private:
        
        CARTrackedObjectPluginFacetPool m_ARTrackedObjectPluginFacets;
    };
} // namespace

namespace
{
    CDtARTrackedObjectManager::CDtARTrackedObjectManager()
        : m_ARTrackedObjectPluginFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtARTrackedObjectManager::~CDtARTrackedObjectManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtARTrackedObjectManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtARTrackedObjectManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtARTrackedObjectManager::Clear()
    {
        m_ARTrackedObjectPluginFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginFacet* CDtARTrackedObjectManager::CreateARTrackedObjectPlugin()
    {
        CInternARTrackedObjectPluginFacet& rDataPluginFacet = m_ARTrackedObjectPluginFacets.Allocate();

        return &rDataPluginFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtARTrackedObjectManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace ARTrackedObjectManager
{
    void OnStart()
    {
        CDtARTrackedObjectManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtARTrackedObjectManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtARTrackedObjectManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginFacet* CreateARTrackedObjectPlugin()
    {
        return CDtARTrackedObjectManager::GetInstance().CreateARTrackedObjectPlugin();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtARTrackedObjectManager::GetInstance().Update();
    }
} // namespace PluginManager
} // namespace Dt
