
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_post_aa_facet.h"
#include "data/data_post_aa_manager.h"

using namespace Dt;
using namespace Dt::PostAAManager;

namespace
{
    class CDtPostAAManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtPostAAManager);
        
    public:
        
        CDtPostAAManager();
        ~CDtPostAAManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CPostAAFXFacet* CreatePostAAFX();

        void Update();
        
    private:

        class CInternPostAAFXFacet : public CPostAAFXFacet
        {
        private:
            friend class CDtPostAAManager;
        };
        
    private:

        typedef Base::CPool<CInternPostAAFXFacet, 2> CPostAAFXFacetPool;

    private:
        
        CPostAAFXFacetPool m_PostAAFXFacets;
    };
} // namespace

namespace
{
    CDtPostAAManager::CDtPostAAManager()
        : m_PostAAFXFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtPostAAManager::~CDtPostAAManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtPostAAManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtPostAAManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtPostAAManager::Clear()
    {
        m_PostAAFXFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CPostAAFXFacet* CDtPostAAManager::CreatePostAAFX()
    {
        CInternPostAAFXFacet& rDataFXAAFXFacet = m_PostAAFXFacets.Allocate();

        return &rDataFXAAFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtPostAAManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace PostAAManager
{
    void OnStart()
    {
        CDtPostAAManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtPostAAManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtPostAAManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CPostAAFXFacet* CreatePostAAFX()
    {
        return CDtPostAAManager::GetInstance().CreatePostAAFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtPostAAManager::GetInstance().Update();
    }
} // namespace PostAAManager
} // namespace Dt
