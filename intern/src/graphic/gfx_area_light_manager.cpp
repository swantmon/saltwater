
#include "graphic/gfx_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_area_light_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_type.h"

#include "graphic/gfx_area_light_facet.h"
#include "graphic/gfx_area_light_manager.h"
#include "graphic/gfx_buffer_manager.h"

using namespace Gfx;

namespace 
{
    class CGfxAreaLightManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxAreaLightManager)

    public:

        CGfxAreaLightManager();
        ~CGfxAreaLightManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

    private:

        class CInternAreaLightFacet : public CAreaLightFacet
        {
        public:

            CInternAreaLightFacet();
            ~CInternAreaLightFacet();

        private:

            friend class CGfxAreaLightManager;
        };

    private:

        typedef Base::CPool<CInternAreaLightFacet, 64> CAreaLightFacets;

    private:

        CAreaLightFacets m_AreaLightFacets;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternAreaLightFacet& AllocateAreaLightFacet();
    };
} // namespace 

namespace 
{
    CGfxAreaLightManager::CInternAreaLightFacet::CInternAreaLightFacet()
        : CAreaLightFacet   ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxAreaLightManager::CInternAreaLightFacet::~CInternAreaLightFacet()
    {
    }
} // namespace 

namespace 
{
    CGfxAreaLightManager::CGfxAreaLightManager()
        : m_AreaLightFacets()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxAreaLightManager::~CGfxAreaLightManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxAreaLightManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnExit()
    {
        m_AreaLightFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::Update()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        CInternAreaLightFacet* pGfxLightFacet = 0;

        assert(_pEntity != 0);

        // -----------------------------------------------------------------------------
        // Entity check
        // -----------------------------------------------------------------------------
        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;
        if (_pEntity->GetType()     != Dt::SLightType::Area) return;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        Dt::CAreaLightFacet* pDtLightFacet = static_cast<Dt::CAreaLightFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        if (pDtLightFacet == nullptr) return;

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            CInternAreaLightFacet& rGfxLightFacet = AllocateAreaLightFacet();

            // -----------------------------------------------------------------------------
            // Create data
            // -----------------------------------------------------------------------------
            SBufferDescriptor BufferDesc;

            static float PlaneVertexBufferData[] =
            {
                0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,
            };
        
            static unsigned int PlaneIndexBufferData[] =
            {
                0, 1, 2, 0, 2, 3,
            };
        
            // -----------------------------------------------------------------------------
            // Engine buffer handling
            // -----------------------------------------------------------------------------
            BufferDesc.m_Stride        = 0;
            BufferDesc.m_Usage         = CBuffer::GPURead;
            BufferDesc.m_Binding       = CBuffer::VertexBuffer;
            BufferDesc.m_Access        = CBuffer::CPUWrite;
            BufferDesc.m_NumberOfBytes = sizeof(PlaneVertexBufferData);
            BufferDesc.m_pBytes        = &PlaneVertexBufferData[0];
            BufferDesc.m_pClassKey     = 0;
        
            CBufferPtr PlanePositionBuffer = BufferManager::CreateBuffer(BufferDesc);
        
            rGfxLightFacet.m_PlaneVertexBufferSetPtr = BufferManager::CreateVertexBufferSet(PlanePositionBuffer);
        
            // -----------------------------------------------------------------------------
        
            BufferDesc.m_Stride        = 0;
            BufferDesc.m_Usage         = CBuffer::GPURead;
            BufferDesc.m_Binding       = CBuffer::IndexBuffer;
            BufferDesc.m_Access        = CBuffer::CPUWrite;
            BufferDesc.m_NumberOfBytes = sizeof(PlaneIndexBufferData);
            BufferDesc.m_pBytes        = &PlaneIndexBufferData[0];
            BufferDesc.m_pClassKey     = 0;
        
            rGfxLightFacet.m_PlaneIndexBufferPtr = BufferManager::CreateBuffer(BufferDesc);

            // -----------------------------------------------------------------------------
            // Save facet
            // -----------------------------------------------------------------------------
            _pEntity->SetDetailFacet(Dt::SFacetCategory::Graphic, &rGfxLightFacet);

            // -----------------------------------------------------------------------------
            // pGfxPointLightFacet
            // -----------------------------------------------------------------------------
            pGfxLightFacet = &rGfxLightFacet;
        }
        else
        {
            pGfxLightFacet = static_cast<CInternAreaLightFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));
        }
        
        assert(pGfxLightFacet);

        // -----------------------------------------------------------------------------
        // Update
        // -----------------------------------------------------------------------------
        Base::Float3 LightPosition  = _pEntity->GetWorldPosition();
        Base::Float3 LightDirection = pDtLightFacet->GetDirection().Normalize() * Base::Float3(-1.0f);
        Base::Float3 DirectionX     = Base::Float3(0.0f, pDtLightFacet->GetRotation(), 1.0f).Normalize();
        Base::Float3 DirectionY     = LightDirection.CrossProduct(DirectionX).Normalize();

        DirectionX = LightDirection.CrossProduct(DirectionY);

        pGfxLightFacet->m_DirectionX = Base::Float4(DirectionY, 0.0f);
        pGfxLightFacet->m_DirectionY = Base::Float4(DirectionX, 0.0f);
        pGfxLightFacet->m_HalfWidth  = 0.5f * pDtLightFacet->GetWidth();
        pGfxLightFacet->m_HalfHeight = 0.5f * pDtLightFacet->GetHeight();
        pGfxLightFacet->m_Plane      = Base::Float4(LightDirection, -(LightDirection.DotProduct(LightPosition)));

        Base::Float3 ExtendX = Base::Float3(pDtLightFacet->GetWidth()  * 0.5f) * DirectionY;
        Base::Float3 ExtendY = Base::Float3(pDtLightFacet->GetHeight() * 0.5f) * DirectionX;

        Base::Float3 LightbulbCorners0 = LightPosition - ExtendX - ExtendY;
        Base::Float3 LightbulbCorners1 = LightPosition + ExtendX - ExtendY;
        Base::Float3 LightbulbCorners2 = LightPosition + ExtendX + ExtendY;
        Base::Float3 LightbulbCorners3 = LightPosition - ExtendX + ExtendY;

        float ViewBuffer[12];

        ViewBuffer[0] = LightbulbCorners0[0];
        ViewBuffer[1] = LightbulbCorners0[1];
        ViewBuffer[2] = LightbulbCorners0[2];

        ViewBuffer[3] = LightbulbCorners1[0];
        ViewBuffer[4] = LightbulbCorners1[1];
        ViewBuffer[5] = LightbulbCorners1[2];

        ViewBuffer[6] = LightbulbCorners2[0];
        ViewBuffer[7] = LightbulbCorners2[1];
        ViewBuffer[8] = LightbulbCorners2[2];

        ViewBuffer[9] = LightbulbCorners3[0];
        ViewBuffer[10] = LightbulbCorners3[1];
        ViewBuffer[11] = LightbulbCorners3[2];

        BufferManager::UploadVertexBufferData(pGfxLightFacet->m_PlaneVertexBufferSetPtr->GetBuffer(0), ViewBuffer);

        // -----------------------------------------------------------------------------
        // Set time
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGfxLightFacet->m_TimeStamp = FrameTime;
    }

    // -----------------------------------------------------------------------------

    CGfxAreaLightManager::CInternAreaLightFacet& CGfxAreaLightManager::AllocateAreaLightFacet()
    {
        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternAreaLightFacet& rGfxLightFacet = m_AreaLightFacets.Allocate();
        
        return rGfxLightFacet;
    }
} // namespace 

namespace Gfx
{
namespace AreaLightManager
{
    void OnStart()
    {
        CGfxAreaLightManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxAreaLightManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxAreaLightManager::GetInstance().Update();
    }
} // namespace AreaLightManager
} // namespace Gfx