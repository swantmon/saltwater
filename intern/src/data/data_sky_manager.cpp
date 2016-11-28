
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_sky_facet.h"
#include "data/data_sky_manager.h"

using namespace Dt;
using namespace Dt::SkyManager;

namespace
{
    class CDtSkyManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtSkyManager);
        
    public:
        
        CDtSkyManager();
        ~CDtSkyManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CSkyFacet* CreateSky();

        void Update();
        
    private:
        
        class CInternSkyboxFacet : public CSkyFacet
        {
        private:
            friend class CDtSkyManager;
        };
        
    private:
        
        typedef Base::CPool<CInternSkyboxFacet,    1> CSkyboxFacetPool;

    private:

        CSkyboxFacetPool m_SkyboxFacets;
    };
} // namespace

namespace
{
    CDtSkyManager::CDtSkyManager()
        : m_SkyboxFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtSkyManager::~CDtSkyManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSkyManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSkyManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtSkyManager::Clear()
    {
        m_SkyboxFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CSkyFacet* CDtSkyManager::CreateSky()
    {
        CInternSkyboxFacet& rDataSkyboxFacet = m_SkyboxFacets.Allocate();

        return &rDataSkyboxFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtSkyManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace SkyManager
{
    void OnStart()
    {
        CDtSkyManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtSkyManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtSkyManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CSkyFacet* CreateSky()
    {
        return CDtSkyManager::GetInstance().CreateSky();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtSkyManager::GetInstance().Update();
    }
} // namespace SkyManager
} // namespace Dt
