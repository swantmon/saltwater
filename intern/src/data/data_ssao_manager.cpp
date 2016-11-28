
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ssao_facet.h"
#include "data/data_ssao_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::SSAOManager;

namespace
{
    class CDtSSAOManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtSSAOManager);
        
    public:
        
        CDtSSAOManager();
        ~CDtSSAOManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CSSAOFXFacet* CreateSSAOFX();

        void Update();
        
    private:
        
        class CInternSSAOFXFacet : public CSSAOFXFacet
        {
        private:
            friend class CDtSSAOManager;
        };
        
    private:
        
        typedef Base::CPool<CInternSSAOFXFacet, 2> CSSAOFXFacetPool;

    private:
        
        CSSAOFXFacetPool m_SSAOFXFacets;
    };
} // namespace

namespace
{
    CDtSSAOManager::CDtSSAOManager()
        : m_SSAOFXFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtSSAOManager::~CDtSSAOManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSSAOManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtSSAOManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtSSAOManager::Clear()
    {
        m_SSAOFXFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CSSAOFXFacet* CDtSSAOManager::CreateSSAOFX()
    {
        CInternSSAOFXFacet& rDataSSAOFXFacet = m_SSAOFXFacets.Allocate();

        return &rDataSSAOFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtSSAOManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace SSAOManager
{
    void OnStart()
    {
        CDtSSAOManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtSSAOManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtSSAOManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CSSAOFXFacet* CreateSSAOFX()
    {
        return CDtSSAOManager::GetInstance().CreateSSAOFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtSSAOManager::GetInstance().Update();
    }
} // namespace SSAOManager
} // namespace Dt
