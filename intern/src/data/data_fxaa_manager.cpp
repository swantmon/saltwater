
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_fxaa_facet.h"
#include "data/data_fxaa_manager.h"

using namespace Dt;
using namespace Dt::FXAAManager;

namespace
{
    class CDtFXAAManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtFXAAManager);
        
    public:
        
        CDtFXAAManager();
        ~CDtFXAAManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CFXAAFXFacet* CreateFXAAFX();

        void Update();
        
    private:

        class CInternFXAAFXFacet : public CFXAAFXFacet
        {
        private:
            friend class CDtFXAAManager;
        };
        
    private:

        typedef Base::CPool<CInternFXAAFXFacet, 2> CFXAAFXFacetPool;

    private:
        
        CFXAAFXFacetPool m_FXAAFXFacets;
    };
} // namespace

namespace
{
    CDtFXAAManager::CDtFXAAManager()
        : m_FXAAFXFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtFXAAManager::~CDtFXAAManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtFXAAManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtFXAAManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtFXAAManager::Clear()
    {
        m_FXAAFXFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CFXAAFXFacet* CDtFXAAManager::CreateFXAAFX()
    {
        CInternFXAAFXFacet& rDataFXAAFXFacet = m_FXAAFXFacets.Allocate();

        return &rDataFXAAFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtFXAAManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace FXAAManager
{
    void OnStart()
    {
        CDtFXAAManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtFXAAManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtFXAAManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CFXAAFXFacet* CreateFXAAFX()
    {
        return CDtFXAAManager::GetInstance().CreateFXAAFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtFXAAManager::GetInstance().Update();
    }
} // namespace FXAAManager
} // namespace Dt
