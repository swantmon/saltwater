
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_light_facet.h"
#include "data/data_light_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::LightManager;

namespace
{
    class CDtLightManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtLightManager);
        
    public:
        
        CDtLightManager();
        ~CDtLightManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CPointLightFacet* CreatePointLight();
	    CSunLightFacet* CreateSunLight();
	    CGlobalProbeLightFacet* CreateGlobalProbeLight();
	    CSkyboxFacet* CreateSkybox();

        void Update();
        
    private:
        
        class CInternPointLightFacet : public CPointLightFacet
        {
        private:
            friend class CDtLightManager;
        };

        class CInternSunLightFacet : public CSunLightFacet
        {
        private:
            friend class CDtLightManager;
        };

        class CInternGlobalProbeLightFacet : public CGlobalProbeLightFacet
        {
        private:
            friend class CDtLightManager;
        };

        class CInternSkyboxFacet : public CSkyboxFacet
        {
        private:
            friend class CDtLightManager;
        };
        
    private:
        
        typedef Base::CPool<CInternPointLightFacet      , 2048> CPointLightFacetPool;
        typedef Base::CPool<CInternSunLightFacet        ,    1> CSunLightFacetPool;
        typedef Base::CPool<CInternGlobalProbeLightFacet,   16> CGlobalProbeLightFacetPool;
        typedef Base::CPool<CInternSkyboxFacet          ,    1> CSkyboxFacetPool;

    private:
        
        CPointLightFacetPool       m_PointLightFacets;
        CSunLightFacetPool         m_SunLightFacets;
        CGlobalProbeLightFacetPool m_GlobalProbeLightFacets;
        CSkyboxFacetPool           m_SkyboxFacets;
    };
} // namespace

namespace
{
    CDtLightManager::CDtLightManager()
        : m_PointLightFacets      ()
        , m_SunLightFacets        ()
        , m_GlobalProbeLightFacets()
        , m_SkyboxFacets          ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtLightManager::~CDtLightManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtLightManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtLightManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtLightManager::Clear()
    {
        m_PointLightFacets      .Clear();
        m_SunLightFacets        .Clear();
        m_GlobalProbeLightFacets.Clear();
        m_SkyboxFacets          .Clear();
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet* CDtLightManager::CreatePointLight()
    {
        CInternPointLightFacet& rDataPointLightFacet = m_PointLightFacets.Allocate();

        return &rDataPointLightFacet;
    }

    // -----------------------------------------------------------------------------

    CSunLightFacet* CDtLightManager::CreateSunLight()
    {
        CInternSunLightFacet& rDataSunLightFacet = m_SunLightFacets.Allocate();

        return &rDataSunLightFacet;
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet* CDtLightManager::CreateGlobalProbeLight()
    {
        CInternGlobalProbeLightFacet& rDataGlobalProbeLightFacet = m_GlobalProbeLightFacets.Allocate();

        return &rDataGlobalProbeLightFacet;
    }

    // -----------------------------------------------------------------------------

    CSkyboxFacet* CDtLightManager::CreateSkybox()
    {
        CInternSkyboxFacet& rDataSkyboxFacet = m_SkyboxFacets.Allocate();

        return &rDataSkyboxFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtLightManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace LightManager
{
    void OnStart()
    {
        CDtLightManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtLightManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtLightManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet* CreatePointLight()
    {
        return CDtLightManager::GetInstance().CreatePointLight();
    }

    // -----------------------------------------------------------------------------

    CSunLightFacet* CreateSunLight()
    {
        return CDtLightManager::GetInstance().CreateSunLight();
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet* CreateGlobalProbeLight()
    {
        return CDtLightManager::GetInstance().CreateGlobalProbeLight();
    }

    // -----------------------------------------------------------------------------

    CSkyboxFacet* CreateSkybox()
    {
        return CDtLightManager::GetInstance().CreateSkybox();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtLightManager::GetInstance().Update();
    }
} // namespace LightManager
} // namespace Dt
