
#include "engine/engine_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_managed_pool.h"

#include "engine/core/core_time.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_area_light_component.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"

#include "engine/graphic/gfx_area_light.h"
#include "engine/graphic/gfx_area_light_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"

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

        class CInternObject : public CAreaLight
        {
        public:

            CInternObject();
            ~CInternObject();

        private:

            friend class CGfxAreaLightManager;
        };

        struct SFilterProperties
        {
            glm::vec4 m_InverseSizeAndOffset;
        };

        struct SBlurProperties
        {
            glm::uvec2   m_Direction;
            unsigned int m_LOD;
        };

    private:

        typedef Base::CManagedPool<CInternObject, 4, 0> CAreaLights;

    private:

        CAreaLights m_AreaLights;
        
        CShaderPtr  m_FilterShaderPtr;
        CShaderPtr  m_BackgroundBlurShaderPtr;
        CShaderPtr  m_CombineShaderPtr;
        CShaderPtr  m_ForegroundBlurShaderPtr;
        CTexturePtr m_BackgroundTexturePtr;
        CBufferPtr  m_GaussianPropertiesPtr;
		CBufferPtr  m_FilterPropertiesPtr;

		Dt::CComponentManager::CComponentDelegate::HandleType m_DirtyComponentDelegate;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void FilterTexture(Gfx::CTexturePtr _TexturePtr, Gfx::CTexturePtr _OutputTexturePtr);
    };
} // namespace 

namespace 
{
    CGfxAreaLightManager::CInternObject::CInternObject()
        : CAreaLight()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxAreaLightManager::CInternObject::~CInternObject()
    {
    }
} // namespace 

namespace 
{
    CGfxAreaLightManager::CGfxAreaLightManager()
        : m_AreaLights             ( )
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

        m_FilterShaderPtr = ShaderManager::CompileCS("area_light/cs_light_arealight_filter.glsl", "Filter", "#define TILE_SIZE 1\n");

        m_BackgroundBlurShaderPtr = ShaderManager::CompileCS("area_light/cs_light_arealight_filter.glsl", "BlurBackground", "#define TILE_SIZE 8\n");

        m_ForegroundBlurShaderPtr = ShaderManager::CompileCS("area_light/cs_light_arealight_filter.glsl", "BlurForeground", "#define TILE_SIZE 8\n");

        m_CombineShaderPtr = ShaderManager::CompileCS("area_light/cs_light_arealight_filter.glsl", "Combine", "#define TILE_SIZE 1\n");

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

        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
		m_DirtyComponentDelegate = Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(std::bind(&CGfxAreaLightManager::OnDirtyComponent, this, std::placeholders::_1));

        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxAreaLightManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnExit()
    {
        m_AreaLights.Clear();

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
        if (_pEntity->GetDirtyFlags() != Dt::CEntity::DirtyMove) return;

        auto ComponentFacet = _pEntity->GetComponentFacet();

        if (!ComponentFacet->HasComponent<Dt::CAreaLightComponent>()) return;

        auto AreaLightComponents = ComponentFacet->GetComponents();

        for (auto pComponent : AreaLightComponents)
        {
            if (pComponent->GetTypeID() == Base::CTypeInfo::GetTypeID<Dt::CAreaLightComponent>())
            {
                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CAreaLightComponent::DirtyInfo);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Vars
        // -----------------------------------------------------------------------------
        CInternObject* pGfxLightFacet = 0;

        // -----------------------------------------------------------------------------
        // Only if component has changed
        // -----------------------------------------------------------------------------
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CAreaLightComponent>()) return;

        auto pAreaLightComponent = static_cast<Dt::CAreaLightComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pAreaLightComponent->GetDirtyFlags();

        if ((DirtyFlags & Dt::CAreaLightComponent::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            pGfxLightFacet = m_AreaLights.Allocate();

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
        
            pGfxLightFacet->m_PlaneVertexBufferSetPtr = BufferManager::CreateBuffer(BufferDesc);
        
            // -----------------------------------------------------------------------------
        
            BufferDesc.m_Stride        = 0;
            BufferDesc.m_Usage         = CBuffer::GPURead;
            BufferDesc.m_Binding       = CBuffer::IndexBuffer;
            BufferDesc.m_Access        = CBuffer::CPUWrite;
            BufferDesc.m_NumberOfBytes = sizeof(PlaneIndexBufferData);
            BufferDesc.m_pBytes        = &PlaneIndexBufferData[0];
            BufferDesc.m_pClassKey     = 0;
        
            pGfxLightFacet->m_PlaneIndexBufferPtr = BufferManager::CreateBuffer(BufferDesc);

            // -----------------------------------------------------------------------------
            // Texture
            // -----------------------------------------------------------------------------
            pGfxLightFacet->m_FilteredTexturePtr = 0;
            pGfxLightFacet->m_TexturePtr         = 0;

            // -----------------------------------------------------------------------------
            // Link
            // -----------------------------------------------------------------------------
            pAreaLightComponent->SetFacet(Dt::CAreaLightComponent::Graphic, pGfxLightFacet);
        }
        else
        {
            pGfxLightFacet = static_cast<CInternObject*>(pAreaLightComponent->GetFacet(Dt::CAreaLightComponent::Graphic));

            if (pAreaLightComponent->GetHasTexture())
            {
                STextureDescriptor TextureDescriptor;

                TextureDescriptor.m_NumberOfPixelsU  = STextureDescriptor::s_NumberOfPixelsFromSource;
                TextureDescriptor.m_NumberOfPixelsV  = STextureDescriptor::s_NumberOfPixelsFromSource;
                TextureDescriptor.m_NumberOfPixelsW  = STextureDescriptor::s_NumberOfPixelsFromSource;
                TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
                TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_NumberOfTexturesFromSource;
                TextureDescriptor.m_Binding          = CTexture::ShaderResource;
                TextureDescriptor.m_Access           = CTexture::CPUWrite;
                TextureDescriptor.m_Format           = CTexture::Unknown;
                TextureDescriptor.m_Usage            = CTexture::GPURead;
                TextureDescriptor.m_Semantic         = CTexture::Diffuse;
                TextureDescriptor.m_pPixels          = 0;
                TextureDescriptor.m_Format           = STextureDescriptor::s_FormatFromSource;
                TextureDescriptor.m_pFileName        = pAreaLightComponent->GetTexture().c_str();

                Gfx::CTexturePtr GfxTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

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
            else
            {
                pGfxLightFacet->m_TexturePtr = 0;
            }
        }
        
        assert(pGfxLightFacet);

        // -----------------------------------------------------------------------------
        // Update
        // -----------------------------------------------------------------------------
        glm::vec3 LightPosition  = pAreaLightComponent->GetHostEntity()->GetWorldPosition();
        glm::vec3 LightDirection = glm::normalize(pAreaLightComponent->GetDirection()) * glm::vec3(-1.0f);
        glm::vec3 DirectionX     = glm::normalize(glm::vec3(0.0f, pAreaLightComponent->GetRotation(), 1.0f));
        glm::vec3 DirectionY     = glm::cross(LightDirection, glm::normalize(DirectionX));

        DirectionX = glm::cross(LightDirection, DirectionY);

        pGfxLightFacet->m_DirectionX = glm::vec4(DirectionY, 0.0f);
        pGfxLightFacet->m_DirectionY = glm::vec4(DirectionX, 0.0f);
        pGfxLightFacet->m_HalfWidth  = 0.5f * pAreaLightComponent->GetWidth();
        pGfxLightFacet->m_HalfHeight = 0.5f * pAreaLightComponent->GetHeight();
        pGfxLightFacet->m_Plane      = glm::vec4(LightDirection, -(glm::dot(LightDirection, LightPosition)));

        glm::vec3 ExtendX = glm::vec3(pAreaLightComponent->GetWidth()  * 0.5f) * DirectionY;
        glm::vec3 ExtendY = glm::vec3(pAreaLightComponent->GetHeight() * 0.5f) * DirectionX;

        glm::vec3 LightbulbCorners0 = LightPosition - ExtendX - ExtendY;
        glm::vec3 LightbulbCorners1 = LightPosition + ExtendX - ExtendY;
        glm::vec3 LightbulbCorners2 = LightPosition + ExtendX + ExtendY;
        glm::vec3 LightbulbCorners3 = LightPosition - ExtendX + ExtendY;

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

    void CGfxAreaLightManager::FilterTexture(Gfx::CTexturePtr _TexturePtr, Gfx::CTexturePtr _OutputTexturePtr)
    {
        // -----------------------------------------------------------------------------
        // Filter outer area with linear clamp filtering
        // -----------------------------------------------------------------------------
        SFilterProperties FilterSettings;

        FilterSettings.m_InverseSizeAndOffset = glm::vec4(1.0f / static_cast<float>(m_BackgroundTexturePtr->GetNumberOfPixelsU()), 1.0f / static_cast<float>(m_BackgroundTexturePtr->GetNumberOfPixelsV()), 0.125f, 0.125f);

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

        FilterSettings.m_InverseSizeAndOffset = glm::vec4(1.0f / static_cast<float>(MipmapLevel0Ptr->GetNumberOfPixelsU()), 1.0f / static_cast<float>(MipmapLevel0Ptr->GetNumberOfPixelsV()), 0.125f, 0.125f);

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

            FilterSettings.m_InverseSizeAndOffset = glm::vec4(1.0f / static_cast<float>(CurrentMipmapLevel->GetNumberOfPixelsU()), 1.0f / static_cast<float>(CurrentMipmapLevel->GetNumberOfPixelsV()), 0.125f, 0.125f);

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