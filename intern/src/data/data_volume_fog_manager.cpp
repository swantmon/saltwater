
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_volume_fog_facet.h"
#include "data/data_volume_fog_manager.h"

using namespace Dt;
using namespace Dt::VolumeFogManager;

namespace
{
    class CDtVolumeFogManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtVolumeFogManager);
        
    public:
        
        CDtVolumeFogManager();
        ~CDtVolumeFogManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CVolumeFogFXFacet* CreateVolumeFogFX();

        void Update();
        
    private:

        class CInternVolumeFogFXFacet : public CVolumeFogFXFacet
        {
        private:
            friend class CDtVolumeFogManager;
        };
        
    private:

        typedef Base::CPool<CInternVolumeFogFXFacet, 2> CVolumeFogFXFacetPool;

    private:

        CVolumeFogFXFacetPool m_VolumeFogFXFacetPool;
    };
} // namespace

namespace
{
    CDtVolumeFogManager::CDtVolumeFogManager()
        : m_VolumeFogFXFacetPool()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtVolumeFogManager::~CDtVolumeFogManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtVolumeFogManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtVolumeFogManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtVolumeFogManager::Clear()
    {
        m_VolumeFogFXFacetPool.Clear();
    }

    // -----------------------------------------------------------------------------

    CVolumeFogFXFacet* CDtVolumeFogManager::CreateVolumeFogFX()
    {
        CInternVolumeFogFXFacet& rDataVolumeFogFXFacet = m_VolumeFogFXFacetPool.Allocate();

        return &rDataVolumeFogFXFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtVolumeFogManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace VolumeFogManager
{
    void OnStart()
    {
        CDtVolumeFogManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtVolumeFogManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtVolumeFogManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CVolumeFogFXFacet* CreateVolumeFogFX()
    {
        return CDtVolumeFogManager::GetInstance().CreateVolumeFogFX();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtVolumeFogManager::GetInstance().Update();
    }
} // namespace VolumeFogManager
} // namespace Dt
