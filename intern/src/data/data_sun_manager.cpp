
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_sun_facet.h"
#include "data/data_sun_manager.h"


using namespace Dt;
using namespace Dt::SunManager;

namespace
{
    class CDtSunManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtSunManager);
        
    public:
        
        CDtSunManager();
        ~CDtSunManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CSunLightFacet* CreateSunLight();

        void Update();
        
    private:
        
        class CInternSunLightFacet : public CSunLightFacet
        {
        private:
            friend class CDtSunManager;
        };
        
    private:
        
        typedef Base::CPool<CInternSunLightFacet, 1> CSunLightFacetPool;

    private:
        
        CSunLightFacetPool m_SunLightFacets;
    };
} // namespace

namespace
{
    CDtSunManager::CDtSunManager()
        : m_SunLightFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtSunManager::~CDtSunManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSunManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSunManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtSunManager::Clear()
    {
        m_SunLightFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CSunLightFacet* CDtSunManager::CreateSunLight()
    {
        CInternSunLightFacet& rDataSunLightFacet = m_SunLightFacets.Allocate();

        return &rDataSunLightFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtSunManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace SunManager
{
    void OnStart()
    {
        CDtSunManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtSunManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtSunManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CSunLightFacet* CreateSunLight()
    {
        return CDtSunManager::GetInstance().CreateSunLight();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtSunManager::GetInstance().Update();
    }
} // namespace SunManager
} // namespace Dt
