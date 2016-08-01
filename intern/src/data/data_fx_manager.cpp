
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_fx_facet.h"
#include "data/data_fx_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::FXManager;

namespace
{
    class CDtFXManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtFXManager);
        
    public:
        
        CDtFXManager();
        ~CDtFXManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CBloomFXFacet* CreateBloomFX();
        CSSRFXFacet* CreateSSRFX();
        CDOFFXFacet* CreateDOFFX();
        CFXAAFXFacet* CreateFXAAFX();
        CSSAOFXFacet* CreateSSAOFX();
        CVolumeFogFXFacet* CreateVolumeFogFX();

        void Update();
        
    private:
        
        class CInternBloomFXFacet : public CBloomFXFacet
        {
        private:
            friend class CDtFXManager;
        };

        class CInternSSRFXFacet : public CSSRFXFacet
        {
        private:
            friend class CDtFXManager;
        };

        class CInternDOFFXFacet : public CDOFFXFacet
        {
        private:
            friend class CDtFXManager;
        };

        class CInternFXAAFXFacet : public CFXAAFXFacet
        {
        private:
            friend class CDtFXManager;
        };

        class CInternSSAOFXFacet : public CSSAOFXFacet
        {
        private:
            friend class CDtFXManager;
        };

        class CInternVolumeFogFXFacet : public CVolumeFogFXFacet
        {
        private:
            friend class CDtFXManager;
        };
        
    private:
        
        typedef Base::CPool<CInternBloomFXFacet    , 2> CBloomFXFacetPool;
        typedef Base::CPool<CInternSSRFXFacet      , 2> CSSRFXFacetPool;
        typedef Base::CPool<CInternDOFFXFacet      , 2> CDOFFXFacetPool;
        typedef Base::CPool<CInternFXAAFXFacet     , 2> CFXAAFXFacetPool;
        typedef Base::CPool<CInternSSAOFXFacet     , 2> CSSAOFXFacetPool;
        typedef Base::CPool<CInternVolumeFogFXFacet, 2> CVolumeFogFXFacetPool;

    private:
        
        CBloomFXFacetPool     m_BloomFXFacets;
        CSSRFXFacetPool       m_SSRFXFacets;
        CDOFFXFacetPool       m_DOFFXFacets;
        CFXAAFXFacetPool      m_FXAAFXFacets;
        CSSAOFXFacetPool      m_SSAOFXFacets;
        CVolumeFogFXFacetPool m_VolumeFogFXFacetPool;
    };
} // namespace

namespace
{
    CDtFXManager::CDtFXManager()
        : m_BloomFXFacets()
        , m_SSRFXFacets  ()
        , m_DOFFXFacets  ()
        , m_FXAAFXFacets ()
        , m_SSAOFXFacets ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtFXManager::~CDtFXManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtFXManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtFXManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtFXManager::Clear()
    {
        m_BloomFXFacets.Clear();
        m_SSRFXFacets  .Clear();
        m_DOFFXFacets  .Clear();
        m_FXAAFXFacets .Clear();
        m_SSAOFXFacets .Clear();
    }

    // -----------------------------------------------------------------------------

    CBloomFXFacet* CDtFXManager::CreateBloomFX()
    {
        CInternBloomFXFacet& rDataBloomFXFacet = m_BloomFXFacets.Allocate();

        return &rDataBloomFXFacet;
    }

    // -----------------------------------------------------------------------------

    CSSRFXFacet* CDtFXManager::CreateSSRFX()
    {
        CInternSSRFXFacet& rDataSSRFXFacet = m_SSRFXFacets.Allocate();

        return &rDataSSRFXFacet;
    }

    // -----------------------------------------------------------------------------

    CDOFFXFacet* CDtFXManager::CreateDOFFX()
    {
        CInternDOFFXFacet& rDataDOFFXFacet = m_DOFFXFacets.Allocate();

        return &rDataDOFFXFacet;
    }

    // -----------------------------------------------------------------------------

    CFXAAFXFacet* CDtFXManager::CreateFXAAFX()
    {
        CInternFXAAFXFacet& rDataFXAAFXFacet = m_FXAAFXFacets.Allocate();

        return &rDataFXAAFXFacet;
    }

    // -----------------------------------------------------------------------------

    CSSAOFXFacet* CDtFXManager::CreateSSAOFX()
    {
        CInternSSAOFXFacet& rDataSSAOFXFacet = m_SSAOFXFacets.Allocate();

        return &rDataSSAOFXFacet;
    }

    // -----------------------------------------------------------------------------

    CVolumeFogFXFacet* CDtFXManager::CreateVolumeFogFX()
    {
        CInternVolumeFogFXFacet& rDataVolumeFogFXFacet = m_VolumeFogFXFacetPool.Allocate();

        return &rDataVolumeFogFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtFXManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace FXManager
{
    void OnStart()
    {
        CDtFXManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtFXManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtFXManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CBloomFXFacet* CreateBloomFX()
    {
        return CDtFXManager::GetInstance().CreateBloomFX();
    }

    // -----------------------------------------------------------------------------

    CSSRFXFacet* CreateSSRFX()
    {
        return CDtFXManager::GetInstance().CreateSSRFX();
    }

    // -----------------------------------------------------------------------------

    CDOFFXFacet* CreateDOFFX()
    {
        return CDtFXManager::GetInstance().CreateDOFFX();
    }

    // -----------------------------------------------------------------------------

    CFXAAFXFacet* CreateFXAAFX()
    {
        return CDtFXManager::GetInstance().CreateFXAAFX();
    }

    // -----------------------------------------------------------------------------

    CSSAOFXFacet* CreateSSAOFX()
    {
        return CDtFXManager::GetInstance().CreateSSAOFX();
    }

    // -----------------------------------------------------------------------------

    CVolumeFogFXFacet* CreateVolumeFogFX()
    {
        return CDtFXManager::GetInstance().CreateVolumeFogFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtFXManager::GetInstance().Update();
    }
} // namespace FXManager
} // namespace Dt
