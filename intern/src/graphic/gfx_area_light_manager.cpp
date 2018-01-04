
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
            
        };

        struct SBlurProperties
        {
            Base::UInt2  m_Direction;
            unsigned int m_LOD;
        };

    private:

        typedef Base::CPool<CInternAreaLightFacet, 64> CAreaLightFacets;

    private:

        CAreaLightFacets m_AreaLightFacets;

        CShaderPtr    m_FilterShaderPtr;
        CShaderPtr    m_BackgroundBlurShaderPtr;
        CShaderPtr    m_CombineShaderPtr;
        CShaderPtr    m_ForegroundBlurShaderPtr;
        CTexturePtr m_BackgroundTexturePtr;
        CBufferPtr    m_GaussianPropertiesPtr;
        CBufferPtr    m_FilterPropertiesPtr;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternAreaLightFacet& AllocateAreaLightFacet();

        void FilterTexture(Gfx::CTexturePtr _TexturePtr, Gfx::CTexturePtr _OutputTexturePtr);
    };
} // namespace 

namespace 
{
    CGfxAreaLightManager::CInternAreaLightFacet::CInternAreaLightFacet()
        : CAreaLightFacet()
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
        : m_AreaLightFacets        ()
        , m_FilterShaderPtr        (0)
        , m_BackgroundBlurShaderPtr(0)
        , m_CombineShaderPtr       (0)
        , m_ForegroundBlurShaderPtr(0)
        , m_BackgroundTexturePtr   (0)
        , m_GaussianPropertiesPtr  (0)
        , m_FilterPropertiesPtr    (0)
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

        TextureDescriptor.m_NumberOfPixelsU  = 256;
        TextureDescriptor.m_NumberOfPixelsV  = 256;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::Unknown;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;
        
        m_BackgroundTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        m_FilterShaderPtr = ShaderManager::CompileCS("cs_light_arealight_filter.glsl", "Filter", "#define TILE_SIZE 1\n");

        m_BackgroundBlurShaderPtr = ShaderManager::CompileCS("cs_light_arealight_filter.glsl", "BlurBackground", "#define TILE_SIZE 8\n");

        m_ForegroundBlurShaderPtr = ShaderManager::CompileCS("cs_light_arealight_filter.glsl", "BlurForeground", "#define TILE_SIZE 8\n");

        m_CombineShaderPtr = ShaderManager::CompileCS("cs_light_arealight_filter.glsl", "Combine", "#define TILE_SIZE 1\n");

        // -----------------------------------------------------------------------------

        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SBlurProperties);
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

        m_FilterShaderPtr         = 0;
        m_BackgroundBlurShaderPtr = 0;
        m_CombineShaderPtr        = 0;
        m_ForegroundBlurShaderPtr = 0;
        m_BackgroundTexturePtr    = 0;
        m_GaussianPropertiesPtr   = 0;
        m_FilterPropertiesPtr     = 0;
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
        
            rGfxLightFacet.m_PlaneVertexBufferSetPtr = BufferManager::CreateBuffer(BufferDesc);
        
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
                    Gfx::CTexturePtr GfxTexturePtr = Gfx::TextureManager::GetTextureByHash(pDtLightFacet->GetTexture()->GetHash());

                    if (GfxTexturePtr != 0 && GfxTexturePtr.IsValid())
                    {
                        // -----------------------------------------------------------------------------
                        // Remove old
                        // -----------------------------------------------------------------------------
                        pGfxLightFacet->m_TexturePtr         = 0;
                        pGfxLightFacet->m_FilteredTexturePtr = 0;

                        // -----------------------------------------------------------------------------
                        // Create new
                        // -----------------------------------------------------------------------------
                        STextureDescriptor TextureDescriptor;

                        TextureDescriptor.m_NumberOfPixelsU  = GfxTexturePtr->GetNumberOfPixelsU();
                        TextureDescriptor.m_NumberOfPixelsV  = GfxTexturePtr->GetNumberOfPixelsV();
                        TextureDescriptor.m_NumberOfPixelsW  = 1;
                        TextureDescriptor.m_NumberOfMipMaps  = 0;
                        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_GenerateAllMipMaps;
                        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
                        TextureDescriptor.m_Access           = CTexture::CPUWrite;
                        TextureDescriptor.m_Format           = CTexture::Unknown;
                        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
                        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
                        TextureDescriptor.m_pFileName        = 0;
                        TextureDescriptor.m_pPixels          = 0;
                        TextureDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;

                        Gfx::CTexturePtr FilteredTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

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

        BufferManager::UploadBufferData(pGfxLightFacet->m_PlaneVertexBufferSetPtr, ViewBuffer);

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

    void CGfxAreaLightManager::FilterTexture(Gfx::CTexturePtr _TexturePtr, Gfx::CTexturePtr _OutputTexturePtr)
    {
        // -----------------------------------------------------------------------------
        // Filter outer area with linear clamp filtering
        // -----------------------------------------------------------------------------
        SFilterProperties FilterSettings;

        FilterSettings.m_InverseSizeAndOffset = Base::Float4(1.0f / static_cast<float>(m_BackgroundTexturePtr->GetNumberOfPixelsU()), 1.0f / static_cast<float>(m_BackgroundTexturePtr->GetNumberOfPixelsV()), 0.125f, 0.125f);

        BufferManager::UploadBufferData(m_FilterPropertiesPtr, &FilterSettings);

        ContextManager::SetShaderCS(m_FilterShaderPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTexturePtr>(_TexturePtr));

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_BackgroundTexturePtr));

        ContextManager::SetResourceBuffer(1, m_FilterPropertiesPtr);

        ContextManager::Dispatch(m_BackgroundTexturePtr->GetNumberOfPixelsU(), m_BackgroundTexturePtr->GetNumberOfPixelsV(), 1);

        ContextManager::ResetResourceBuffer(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetImageTexture(1);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_BackgroundBlurShaderPtr);

        ContextManager::SetResourceBuffer(1, m_FilterPropertiesPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTexturePtr>(m_BackgroundTexturePtr));

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_BackgroundTexturePtr));

        ContextManager::Dispatch(m_BackgroundTexturePtr->GetNumberOfPixelsU() / 8, m_BackgroundTexturePtr->GetNumberOfPixelsV() / 8, 1);

        ContextManager::ResetImageTexture(0);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetResourceBuffer(1);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_BackgroundBlurShaderPtr);

        ContextManager::SetResourceBuffer(1, m_FilterPropertiesPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTexturePtr>(m_BackgroundTexturePtr));

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_BackgroundTexturePtr));

        ContextManager::Dispatch(m_BackgroundTexturePtr->GetNumberOfPixelsU() / 8, m_BackgroundTexturePtr->GetNumberOfPixelsV() / 8, 1);

        ContextManager::ResetImageTexture(0);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetResourceBuffer(1);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------
        // Apply background to first mipmap level without blurring the foreground
        // -----------------------------------------------------------------------------
        CTexturePtr MipmapLevel0Ptr = TextureManager::GetMipmapFromTexture2D(_OutputTexturePtr, 0);

        FilterSettings.m_InverseSizeAndOffset = Base::Float4(1.0f / static_cast<float>(MipmapLevel0Ptr->GetNumberOfPixelsU()), 1.0f / static_cast<float>(MipmapLevel0Ptr->GetNumberOfPixelsV()), 0.125f, 0.125f);

        BufferManager::UploadBufferData(m_FilterPropertiesPtr, &FilterSettings);

        ContextManager::SetShaderCS(m_CombineShaderPtr);

        ContextManager::SetResourceBuffer(1, m_FilterPropertiesPtr);

        ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, static_cast<CTexturePtr>(m_BackgroundTexturePtr));

        ContextManager::SetSampler(1, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(1, static_cast<CTexturePtr>(_TexturePtr));

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(MipmapLevel0Ptr));

        ContextManager::Dispatch(MipmapLevel0Ptr->GetNumberOfPixelsU(), MipmapLevel0Ptr->GetNumberOfPixelsV(), 1);

        ContextManager::ResetImageTexture(0);

        ContextManager::ResetSampler(1);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetTexture(0);

        ContextManager::ResetResourceBuffer(1);

        ContextManager::ResetShaderCS();

        CTexturePtr LastMipmapLevel = MipmapLevel0Ptr;

        // -----------------------------------------------------------------------------
        // Now do this for every next mipmap level but now with a blur of the 
        // inner image
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfMipmap = 1; IndexOfMipmap < _OutputTexturePtr->GetNumberOfMipLevels(); ++IndexOfMipmap)
        {
            CTexturePtr CurrentMipmapLevel = TextureManager::GetMipmapFromTexture2D(_OutputTexturePtr, IndexOfMipmap);

            // -----------------------------------------------------------------------------
            // Blur foreground for every LOD
            // -----------------------------------------------------------------------------
            SBlurProperties GaussianSettings;

            GaussianSettings.m_Direction[0] = 1;
            GaussianSettings.m_Direction[1] = 0;
            GaussianSettings.m_LOD          = IndexOfMipmap - 1;

            BufferManager::UploadBufferData(m_GaussianPropertiesPtr, &GaussianSettings);

            FilterSettings.m_InverseSizeAndOffset = Base::Float4(1.0f / static_cast<float>(CurrentMipmapLevel->GetNumberOfPixelsU()), 1.0f / static_cast<float>(CurrentMipmapLevel->GetNumberOfPixelsV()), 0.125f, 0.125f);

            BufferManager::UploadBufferData(m_FilterPropertiesPtr, &FilterSettings);

            ContextManager::SetShaderCS(m_ForegroundBlurShaderPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianPropertiesPtr);

            ContextManager::SetResourceBuffer(1, m_FilterPropertiesPtr);

            ContextManager::SetTexture(0, static_cast<CTexturePtr>(LastMipmapLevel));

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(CurrentMipmapLevel));

            ContextManager::Dispatch(CurrentMipmapLevel->GetNumberOfPixelsU(), CurrentMipmapLevel->GetNumberOfPixelsV(), 1);

            ContextManager::ResetImageTexture(0);

            ContextManager::ResetSampler(0);

            ContextManager::ResetTexture(0);

            ContextManager::ResetResourceBuffer(1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------

            GaussianSettings.m_Direction[0] = 0;
            GaussianSettings.m_Direction[1] = 1;
            GaussianSettings.m_LOD          = IndexOfMipmap;

            BufferManager::UploadBufferData(m_GaussianPropertiesPtr, &GaussianSettings);

            ContextManager::SetShaderCS(m_ForegroundBlurShaderPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianPropertiesPtr);

            ContextManager::SetResourceBuffer(1, m_FilterPropertiesPtr);

            ContextManager::SetTexture(0, static_cast<CTexturePtr>(CurrentMipmapLevel));

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(CurrentMipmapLevel));

            ContextManager::Dispatch(CurrentMipmapLevel->GetNumberOfPixelsU(), CurrentMipmapLevel->GetNumberOfPixelsV(), 1);

            ContextManager::ResetImageTexture(0);

            ContextManager::ResetSampler(0);

            ContextManager::ResetTexture(0);

            ContextManager::ResetResourceBuffer(1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();

            LastMipmapLevel = CurrentMipmapLevel;
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