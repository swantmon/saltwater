
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

        struct SFilterProperties
        {
            unsigned int m_LOD;
            Base::Float2 m_InverseSize;
            float        m_Offset;
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
        CTexture2DPtr m_TempFilteredTexturePtr;
        CTexture2DPtr m_TempTexturePtr;
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
        , m_TempFilteredTexturePtr(0)
        , m_TempTexturePtr        (0)
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

        TextureDescriptor.m_NumberOfPixelsU  = 64;
        TextureDescriptor.m_NumberOfPixelsV  = 64;
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
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        m_DownSampleTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
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
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        m_LODSampleTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_NumberOfPixelsU  = 2048;
        TextureDescriptor.m_NumberOfPixelsV  = 2048;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
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
        m_TempFilteredTexturePtr = 0;
        m_TempTexturePtr         = 0;
        m_GaussianPropertiesPtr  = 0;
        m_FilterPropertiesPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::Update()
    {
        // TODO: Add filtering only on changing texture; This here is for testing!
        FilterTexture(m_TempTexturePtr, m_TempFilteredTexturePtr);
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
            STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = 2048;
            TextureDescriptor.m_NumberOfPixelsV  = 2048;
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
            TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
            rGfxLightFacet.m_FilteredTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
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
                    pGfxLightFacet->m_TexturePtr = Gfx::TextureManager::GetTexture2DByHash(pDtLightFacet->GetTexture()->GetHash());

                    FilterTexture(pGfxLightFacet->m_TexturePtr, pGfxLightFacet->m_FilteredTexturePtr);

                    // pGfxLightFacet->m_FilteredTexturePtr = m_TempFilteredTexturePtr;
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
        // Filter
        // -----------------------------------------------------------------------------
        SFilterProperties FilterSettings;

        FilterSettings.m_LOD         = 0;
        FilterSettings.m_Offset      = 0.125f;
        FilterSettings.m_InverseSize = Base::Float2(1.0f / 64.0f, 1.0f / 64.0f);

        BufferManager::UploadConstantBufferData(m_FilterPropertiesPtr, &FilterSettings);

        ContextManager::SetShaderCS(m_FilterShaderPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_TexturePtr));

        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

        ContextManager::SetResourceBuffer(0, m_FilterPropertiesPtr);

        ContextManager::Dispatch(64, 64, 1);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetImageTexture(1);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------
        // Blur
        // -----------------------------------------------------------------------------
        SGaussianProperties GaussianSettings;

        GaussianSettings.m_MaxPixelCoord[0] = 64;
        GaussianSettings.m_MaxPixelCoord[1] = 64;
        GaussianSettings.m_Weights[0] = 0.018816f;
        GaussianSettings.m_Weights[1] = 0.034474f;
        GaussianSettings.m_Weights[2] = 0.056577f;
        GaussianSettings.m_Weights[3] = 0.083173f;
        GaussianSettings.m_Weights[4] = 0.109523f;
        GaussianSettings.m_Weights[5] = 0.129188f;
        GaussianSettings.m_Weights[6] = 0.136498f;
        GaussianSettings.m_Direction[0] = 1;
        GaussianSettings.m_Direction[1] = 0;

        BufferManager::UploadConstantBufferData(m_GaussianPropertiesPtr, &GaussianSettings);

        ContextManager::SetShaderCS(m_BlurShaderPtr);

        ContextManager::SetResourceBuffer(0, m_FilterPropertiesPtr);

        ContextManager::SetResourceBuffer(1, m_GaussianPropertiesPtr);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

        ContextManager::Dispatch(64, 64, 1);

        ContextManager::ResetImageTexture(1);

        ContextManager::ResetResourceBuffer(1);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------

        GaussianSettings.m_Direction[0] = 0;
        GaussianSettings.m_Direction[1] = 1;

        BufferManager::UploadConstantBufferData(m_GaussianPropertiesPtr, &GaussianSettings);

        ContextManager::SetShaderCS(m_BlurShaderPtr);

        ContextManager::SetResourceBuffer(0, m_FilterPropertiesPtr);

        ContextManager::SetResourceBuffer(1, m_GaussianPropertiesPtr);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

        ContextManager::Dispatch(64, 64, 1);

        ContextManager::ResetImageTexture(1);

        ContextManager::ResetResourceBuffer(1);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------
        // For every single LOD blur the original image more and more and save to 
        // every single mipmap
        // -----------------------------------------------------------------------------
        for (unsigned int LOD = 0; LOD < 1; ++ LOD)
        {
            // -----------------------------------------------------------------------------
            // Filter
            // -----------------------------------------------------------------------------
            FilterSettings.m_LOD         = 0;
            FilterSettings.m_Offset      = 0.0f;
            FilterSettings.m_InverseSize = Base::Float2(1.0f / 512.0f, 1.0f / 512.0f);

            BufferManager::UploadConstantBufferData(m_FilterPropertiesPtr, &FilterSettings);

            ContextManager::SetShaderCS(m_FilterShaderPtr);

            ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
            ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_TexturePtr));

            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_LODSampleTexturePtr));

            ContextManager::SetResourceBuffer(0, m_FilterPropertiesPtr);

            ContextManager::Dispatch(512, 512, 1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetSampler(0);
            ContextManager::ResetTexture(0);

            ContextManager::ResetImageTexture(1);

            ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------
            // Blur
            // -----------------------------------------------------------------------------
            SGaussianProperties GaussianSettings;

            GaussianSettings.m_MaxPixelCoord[0] = 512;
            GaussianSettings.m_MaxPixelCoord[1] = 512;
            GaussianSettings.m_Weights[0] = 0.018816f;
            GaussianSettings.m_Weights[1] = 0.034474f;
            GaussianSettings.m_Weights[2] = 0.056577f;
            GaussianSettings.m_Weights[3] = 0.083173f;
            GaussianSettings.m_Weights[4] = 0.109523f;
            GaussianSettings.m_Weights[5] = 0.129188f;
            GaussianSettings.m_Weights[6] = 0.136498f;
            GaussianSettings.m_Direction[0] = 1;
            GaussianSettings.m_Direction[1] = 0;

            BufferManager::UploadConstantBufferData(m_GaussianPropertiesPtr, &GaussianSettings);

            SFilterProperties FilterSettings;

            FilterSettings.m_LOD = 0;

            BufferManager::UploadConstantBufferData(m_FilterPropertiesPtr, &FilterSettings);

            ContextManager::SetShaderCS(m_BlurShaderPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianPropertiesPtr);

            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_LODSampleTexturePtr));

            ContextManager::Dispatch(512, 512, 1);

            ContextManager::ResetImageTexture(1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------

            GaussianSettings.m_Direction[0] = 0;
            GaussianSettings.m_Direction[1] = 1;

            BufferManager::UploadConstantBufferData(m_GaussianPropertiesPtr, &m_LODSampleTexturePtr);

            ContextManager::SetShaderCS(m_BlurShaderPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianPropertiesPtr);

            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

            ContextManager::Dispatch(512, 512, 1);

            ContextManager::ResetImageTexture(1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------
            // Apply
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderCS(m_ApplyShaderPtr);

            ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
            ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_DownSampleTexturePtr));

            ContextManager::SetSampler(1, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
            ContextManager::SetTexture(1, static_cast<CTextureBasePtr>(m_LODSampleTexturePtr));

            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(_OutputTexturePtr));

            ContextManager::Dispatch(2048, 2048, 1);

            ContextManager::ResetImageTexture(1);

            ContextManager::ResetSampler(0);
            ContextManager::ResetTexture(0);

            ContextManager::ResetSampler(1);
            ContextManager::ResetTexture(1);

            ContextManager::ResetShaderCS();
        }
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