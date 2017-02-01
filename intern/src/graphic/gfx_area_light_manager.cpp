
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
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_texture_manager.h"

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

        CShaderPtr      m_ShaderPtr;
        CTextureBasePtr m_TempFilteredTexturePtr;
        CTextureBasePtr m_TempTexturePtr;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternAreaLightFacet& AllocateAreaLightFacet();

        void FilterTexture(Gfx::CTextureBasePtr _TexturePtr, Gfx::CTextureBasePtr _OutputTexturePtr);
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
        : m_AreaLightFacets       ()
        , m_ShaderPtr             (0)
        , m_TempFilteredTexturePtr(0)
        , m_TempTexturePtr        (0)
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


        // -----------------------------------------------------------------------------

        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 2048;
        TextureDescriptor.m_NumberOfPixelsV  = 2048;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_NumberOfMipMapsFromSource;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = "textures/LTC/filtered_map.dds";
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        m_TempFilteredTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 2048;
        TextureDescriptor.m_NumberOfPixelsV  = 2048;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_NumberOfMipMapsFromSource;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = "textures/LTC/map.dds";
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        m_TempTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        m_ShaderPtr = ShaderManager::CompileCS("cs_light_arealight_filter.glsl", "main");
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnExit()
    {
        m_AreaLightFacets.Clear();

        m_ShaderPtr              = 0;
        m_TempFilteredTexturePtr = 0;
        m_TempTexturePtr         = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::Update()
    {
        // TODO: Add filtering only on changing texture; This here is for testing!
        // FilterTexture(m_TempTexturePtr, m_TempFilteredTexturePtr);
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
            // Buffer
            // -----------------------------------------------------------------------------
            SBufferDescriptor BufferDesc;

            static float PlaneVertexBufferData[] =
            {
                0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            };
        
            static unsigned int PlaneIndexBufferData[] =
            {
                0, 1, 2, 0, 2, 3,
            };
        
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
            // Texture
            // -----------------------------------------------------------------------------
            rGfxLightFacet.m_FilteredTexturePtr = 0;
            rGfxLightFacet.m_TexturePtr         = 0;

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

            if (pDtLightFacet->GetHasTexture())
            {
                pGfxLightFacet->m_FilteredTexturePtr = m_TempFilteredTexturePtr;
                pGfxLightFacet->m_TexturePtr         = Gfx::TextureManager::GetTexture2DByHash(pDtLightFacet->GetTexture()->GetHash());
            }
            else
            {
                pGfxLightFacet->m_FilteredTexturePtr = 0;
                pGfxLightFacet->m_TexturePtr         = 0;
            }
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

        float ViewBuffer[20];

        ViewBuffer[0] = LightbulbCorners0[0];
        ViewBuffer[1] = LightbulbCorners0[1];
        ViewBuffer[2] = LightbulbCorners0[2];
        ViewBuffer[3] = 0.0f;
        ViewBuffer[4] = 0.0f;

        ViewBuffer[5] = LightbulbCorners1[0];
        ViewBuffer[6] = LightbulbCorners1[1];
        ViewBuffer[7] = LightbulbCorners1[2];
        ViewBuffer[8] = 1.0f;
        ViewBuffer[9] = 0.0f;

        ViewBuffer[10] = LightbulbCorners2[0];
        ViewBuffer[11] = LightbulbCorners2[1];
        ViewBuffer[12] = LightbulbCorners2[2];
        ViewBuffer[13] = 1.0f;
        ViewBuffer[14] = 1.0f;

        ViewBuffer[15] = LightbulbCorners3[0];
        ViewBuffer[16] = LightbulbCorners3[1];
        ViewBuffer[17] = LightbulbCorners3[2];
        ViewBuffer[18] = 0.0f;
        ViewBuffer[19] = 1.0f;

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

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::FilterTexture(Gfx::CTextureBasePtr _TexturePtr, Gfx::CTextureBasePtr _OutputTexturePtr)
    {
        ContextManager::SetShaderCS(m_ShaderPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, _TexturePtr);

        ContextManager::SetImageTexture(1, _OutputTexturePtr);

        ContextManager::Dispatch(2048, 2048, 1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetImageTexture(1);

        ContextManager::ResetShaderCS();
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