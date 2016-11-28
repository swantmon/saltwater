
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ssr_facet.h"
#include "data/data_ssr_manager.h"

using namespace Dt;
using namespace Dt::SSRFXManager;

namespace
{
    class CDtSSRManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtSSRManager);
        
    public:
        
        CDtSSRManager();
        ~CDtSSRManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CSSRFXFacet* CreateSSRFX();

        void Update();
        
    private:
        
        class CInternSSRFXFacet : public CSSRFXFacet
        {
        private:
            friend class CDtSSRManager;
        };
        
    private:
        
        typedef Base::CPool<CInternSSRFXFacet, 2> CSSRFXFacetPool;

    private:
        
        CSSRFXFacetPool m_SSRFXFacets;
    };
} // namespace

namespace
{
    CDtSSRManager::CDtSSRManager()
        : m_SSRFXFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtSSRManager::~CDtSSRManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSSRManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSSRManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtSSRManager::Clear()
    {
        m_SSRFXFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CSSRFXFacet* CDtSSRManager::CreateSSRFX()
    {
        CInternSSRFXFacet& rDataSSRFXFacet = m_SSRFXFacets.Allocate();

        return &rDataSSRFXFacet;
    }
    
    // -----------------------------------------------------------------------------

    void CDtSSRManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace SSRFXManager
{
    void OnStart()
    {
        CDtSSRManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtSSRManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtSSRManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CSSRFXFacet* CreateSSRFX()
    {
        return CDtSSRManager::GetInstance().CreateSSRFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtSSRManager::GetInstance().Update();
    }
} // namespace SSRFXManager
} // namespace Dt
