
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
#include "data/data_map.h"

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

        struct SFilterProperties
        {
            Base::Float4 m_InverseSizeAndOffset;
            unsigned int m_LOD;
        };

        struct SGaussianProperties
        {
            Base::Int2 m_Direction;
            Base::Int2 m_MaxPixelCoord;
            float      m_Weights[7];
        };

    private:

        typedef Base::CPool<CInternAreaLightFacet, 64> CAreaLightFacets;

    private:

        CAreaLightFacets m_AreaLightFacets;

        CShaderPtr    m_FilterShaderPtr;
        CShaderPtr    m_BlurShaderPtr;
        CShaderPtr    m_ApplyShaderPtr;
        CTexture2DPtr m_DownSampleTexturePtr;
        CTexture2DPtr m_LODSampleTexturePtr;
        CBufferPtr    m_GaussianPropertiesPtr;
        CBufferPtr    m_FilterPropertiesPtr;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternAreaLightFacet& AllocateAreaLightFacet();

        void FilterTexture(Gfx::CTexture2DPtr _TexturePtr, Gfx::CTexture2DPtr _OutputTexturePtr);
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
        , m_FilterShaderPtr       (0)
        , m_BlurShaderPtr         (0)
        , m_ApplyShaderPtr        (0)
        , m_DownSampleTexturePtr  (0)
        , m_LODSampleTexturePtr   (0)
        , m_GaussianPropertiesPtr (0)
        , m_FilterPropertiesPtr   (0)
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

        TextureDescriptor.m_NumberOfPixelsU  = 1024;
        TextureDescriptor.m_NumberOfPixelsV  = 1024;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        
        m_DownSampleTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 1024;
        TextureDescriptor.m_NumberOfPixelsV  = 1024;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        
        m_LODSampleTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        m_FilterShaderPtr = ShaderManager::CompileCS("cs_light_arealight_filter.glsl", "main");

        m_BlurShaderPtr = ShaderManager::CompileCS("cs_light_arealight_blur.glsl", "main");

        m_ApplyShaderPtr = ShaderManager::CompileCS("cs_light_arealight_apply.glsl", "main");

        // -----------------------------------------------------------------------------

        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_GaussianPropertiesPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SFilterProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_FilterPropertiesPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnExit()
    {
        m_AreaLightFacets.Clear();

        m_FilterShaderPtr        = 0;
        m_BlurShaderPtr          = 0;
        m_ApplyShaderPtr         = 0;
        m_DownSampleTexturePtr   = 0;
        m_LODSampleTexturePtr    = 0;
        m_GaussianPropertiesPtr  = 0;
        m_FilterPropertiesPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::Update()
    {
        // The following is for testing:
        // return;

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SLightType::Area)
            {
                Dt::CAreaLightFacet*   pDtLightFacet  = static_cast<Dt::CAreaLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                CInternAreaLightFacet* pGfxLightFacet = static_cast<CInternAreaLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                if (pDtLightFacet->GetHasTexture())
                {
                    if (pGfxLightFacet->m_TexturePtr != 0 && pGfxLightFacet->m_FilteredTexturePtr != 0)
                    {
                        FilterTexture(pGfxLightFacet->m_TexturePtr, pGfxLightFacet->m_FilteredTexturePtr);
                    }
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
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
                if (pGfxLightFacet->m_TexturePtr == 0 || pGfxLightFacet->m_TexturePtr != 0 && pGfxLightFacet->m_TexturePtr->GetHash() != pDtLightFacet->GetTexture()->GetHash())
                {
                    Gfx::CTexture2DPtr GfxTexturePtr = Gfx::TextureManager::GetTexture2DByHash(pDtLightFacet->GetTexture()->GetHash());

                    if (GfxTexturePtr != 0 && GfxTexturePtr.IsValid())
                    {
                        STextureDescriptor TextureDescriptor;

                        TextureDescriptor.m_NumberOfPixelsU  = GfxTexturePtr->GetNumberOfPixelsU();
                        TextureDescriptor.m_NumberOfPixelsV  = GfxTexturePtr->GetNumberOfPixelsV();
                        TextureDescriptor.m_NumberOfPixelsW  = 1;
                        TextureDescriptor.m_NumberOfMipMaps  = 0;
                        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_GenerateAllMipMaps;
                        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
                        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
                        TextureDescriptor.m_Format           = CTextureBase::Unknown;
                        TextureDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
                        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
                        TextureDescriptor.m_pFileName        = 0;
                        TextureDescriptor.m_pPixels          = 0;
                        TextureDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;

                        Gfx::CTexture2DPtr FilteredTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

                        FilterTexture(GfxTexturePtr, FilteredTexturePtr);

                        pGfxLightFacet->m_TexturePtr         = GfxTexturePtr;
                        pGfxLightFacet->m_FilteredTexturePtr = FilteredTexturePtr;
                    }
                }
            }
            else
            {
                pGfxLightFacet->m_TexturePtr = 0;
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

    void CGfxAreaLightManager::FilterTexture(Gfx::CTexture2DPtr _TexturePtr, Gfx::CTexture2DPtr _OutputTexturePtr)
    {
        // -----------------------------------------------------------------------------
        // Filter outer area with linear clamp filtering
        // -----------------------------------------------------------------------------
        SFilterProperties FilterSettings;

        FilterSettings.m_LOD                  = 0;
        FilterSettings.m_InverseSizeAndOffset = Base::Float4(1.0f / static_cast<float>(m_DownSampleTexturePtr->GetNumberOfPixelsU()), 1.0f / static_cast<float>(m_DownSampleTexturePtr->GetNumberOfPixelsV()), 0.125f, 0.125f);

        BufferManager::UploadConstantBufferData(m_FilterPropertiesPtr, &FilterSettings);

        ContextManager::SetShaderCS(m_FilterShaderPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_TexturePtr));

        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

        ContextManager::SetResourceBuffer(0, m_FilterPropertiesPtr);

        ContextManager::Dispatch(m_DownSampleTexturePtr->GetNumberOfPixelsU(), m_DownSampleTexturePtr->GetNumberOfPixelsV(), 1);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetImageTexture(1);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------

        FilterSettings.m_LOD = 0;
        FilterSettings.m_InverseSizeAndOffset = Base::Float4(1.0f / static_cast<float>(m_DownSampleTexturePtr->GetNumberOfPixelsU()), 1.0f / static_cast<float>(m_DownSampleTexturePtr->GetNumberOfPixelsV()), 0.125f, 0.125f);

        BufferManager::UploadConstantBufferData(m_FilterPropertiesPtr, &FilterSettings);      

        ContextManager::SetShaderCS(m_BlurShaderPtr);

        ContextManager::SetResourceBuffer(0, m_FilterPropertiesPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_LODSampleTexturePtr));

        ContextManager::Dispatch(m_LODSampleTexturePtr->GetNumberOfPixelsU() / 8, m_LODSampleTexturePtr->GetNumberOfPixelsV() / 8, 1);

        ContextManager::ResetImageTexture(0);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetResourceBuffer(0);

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