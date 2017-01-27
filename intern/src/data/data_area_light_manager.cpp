
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_area_light_facet.h"
#include "data/data_area_light_manager.h"

using namespace Dt;
using namespace Dt::AreaLightManager;

namespace
{
    class CDtAreaLightManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtAreaLightManager);
        
    public:
        
        CDtAreaLightManager();
        ~CDtAreaLightManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CAreaLightFacet* CreateAreaLight();

        void Update();
        
    private:
        
        class CInternAreaLightFacet : public CAreaLightFacet
        {
        private:
            friend class CDtAreaLightManager;
        };
        
    private:
        
        typedef Base::CPool<CInternAreaLightFacet, 2048> CAreaLightFacetPool;

    private:
        
        CAreaLightFacetPool m_AreaLightFacets;
    };
} // namespace

namespace
{
    CDtAreaLightManager::CDtAreaLightManager()
        : m_AreaLightFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtAreaLightManager::~CDtAreaLightManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtAreaLightManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtAreaLightManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtAreaLightManager::Clear()
    {
        m_AreaLightFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CAreaLightFacet* CDtAreaLightManager::CreateAreaLight()
    {
        CInternAreaLightFacet& rDataAreaLightFacet = m_AreaLightFacets.Allocate();

        return &rDataAreaLightFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtAreaLightManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace AreaLightManager
{
    void OnStart()
    {
        CDtAreaLightManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtAreaLightManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtAreaLightManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CAreaLightFacet* CreateAreaLight()
    {
        return CDtAreaLightManager::GetInstance().CreateAreaLight();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtAreaLightManager::GetInstance().Update();
    }
} // namespace AreaLightManager
} // namespace Dt
