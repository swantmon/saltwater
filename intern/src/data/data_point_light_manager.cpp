
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_point_light_facet.h"
#include "data/data_point_light_manager.h"

using namespace Dt;
using namespace Dt::PointLightManager;

namespace
{
    class CDtPointLightManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtPointLightManager);
        
    public:
        
        CDtPointLightManager();
        ~CDtPointLightManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CPointLightFacet* CreatePointLight();

        void Update();
        
    private:
        
        class CInternPointLightFacet : public CPointLightFacet
        {
        private:
            friend class CDtPointLightManager;
        };
        
    private:
        
        typedef Base::CPool<CInternPointLightFacet, 2048> CPointLightFacetPool;

    private:
        
        CPointLightFacetPool m_PointLightFacets;
    };
} // namespace

namespace
{
    CDtPointLightManager::CDtPointLightManager()
        : m_PointLightFacets      ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtPointLightManager::~CDtPointLightManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtPointLightManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtPointLightManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtPointLightManager::Clear()
    {
        m_PointLightFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet* CDtPointLightManager::CreatePointLight()
    {
        CInternPointLightFacet& rDataPointLightFacet = m_PointLightFacets.Allocate();

        return &rDataPointLightFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtPointLightManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace PointLightManager
{
    void OnStart()
    {
        CDtPointLightManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtPointLightManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtPointLightManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet* CreatePointLight()
    {
        return CDtPointLightManager::GetInstance().CreatePointLight();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtPointLightManager::GetInstance().Update();
    }
} // namespace PointLightManager
} // namespace Dt
