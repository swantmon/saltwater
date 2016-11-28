
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_light_probe_facet.h"
#include "data/data_light_probe_manager.h"

using namespace Dt;
using namespace Dt::LightProbeManager;

namespace
{
    class CDtLightProbeManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtLightProbeManager);
        
    public:
        
        CDtLightProbeManager();
        ~CDtLightProbeManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CLightProbeFacet* CreateLightProbe();

        void Update();
        
    private:
        
        class CInterLightProbeFacet : public CLightProbeFacet
        {
        private:
            friend class CDtLightProbeManager;
        };

    private:

        typedef Base::CPool<CInterLightProbeFacet, 16> CGlobalProbeLightFacetPool;

    private:

        CGlobalProbeLightFacetPool m_LightProbeFacets;
    };
} // namespace

namespace
{
    CDtLightProbeManager::CDtLightProbeManager()
        : m_LightProbeFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtLightProbeManager::~CDtLightProbeManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtLightProbeManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtLightProbeManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtLightProbeManager::Clear()
    {
        m_LightProbeFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet* CDtLightProbeManager::CreateLightProbe()
    {
        CInterLightProbeFacet& rDataGlobalProbeLightFacet = m_LightProbeFacets.Allocate();

        return &rDataGlobalProbeLightFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtLightProbeManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace LightProbeManager
{
    void OnStart()
    {
        CDtLightProbeManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtLightProbeManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtLightProbeManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet* CreateLightProbe()
    {
        return CDtLightProbeManager::GetInstance().CreateLightProbe();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtLightProbeManager::GetInstance().Update();
    }
} // namespace LightProbeManager
} // namespace Dt
