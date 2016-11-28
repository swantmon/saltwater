
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_bloom_facet.h"
#include "data/data_bloom_manager.h"

using namespace Dt;
using namespace Dt::BloomManager;

namespace
{
    class CDtBloomManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtBloomManager);
        
    public:
        
        CDtBloomManager();
        ~CDtBloomManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CBloomFXFacet* CreateBloomFX();

        void Update();
        
    private:
        
        class CInternBloomFXFacet : public CBloomFXFacet
        {
        private:
            friend class CDtBloomManager;
        };

    private:
        
        typedef Base::CPool<CInternBloomFXFacet, 2> CBloomFXFacetPool;

    private:
        
        CBloomFXFacetPool m_BloomFXFacets;
    };
} // namespace

namespace
{
    CDtBloomManager::CDtBloomManager()
        : m_BloomFXFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtBloomManager::~CDtBloomManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtBloomManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtBloomManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtBloomManager::Clear()
    {
        m_BloomFXFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CBloomFXFacet* CDtBloomManager::CreateBloomFX()
    {
        CInternBloomFXFacet& rDataBloomFXFacet = m_BloomFXFacets.Allocate();

        return &rDataBloomFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtBloomManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace BloomManager
{
    void OnStart()
    {
        CDtBloomManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtBloomManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtBloomManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CBloomFXFacet* CreateBloomFX()
    {
        return CDtBloomManager::GetInstance().CreateBloomFX();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtBloomManager::GetInstance().Update();
    }
} // namespace BloomManager
} // namespace Dt
