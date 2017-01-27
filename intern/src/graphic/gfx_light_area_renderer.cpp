
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_area_light_facet.h"
#include "data/data_entity.h"
#include "data/data_light_type.h"
#include "data/data_map.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_ltc_look_up_textures.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxAreaLightRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxAreaLightRenderer)
        
    public:
        CGfxAreaLightRenderer();
        ~CGfxAreaLightRenderer();
        
    public:
        void OnStart();
        void OnExit();
        
        void OnSetupShader();
        void OnSetupKernels();
        void OnSetupRenderTargets();
        void OnSetupStates();
        void OnSetupTextures();
        void OnSetupBuffers();
        void OnSetupResources();
        void OnSetupModels();
        void OnSetupEnd();
        
        void OnReload();
        void OnNewMap();
        void OnUnloadMap();
        
        void Update();
        void Render();
        
    private:
                
        struct SAreaLightProperties
        {
            Base::Float4 m_Color;
            Base::Float4 m_Position;
            Base::Float4 m_DirectionX;
            Base::Float4 m_DirectionY;
            Base::Float4 m_Plane;
            float        m_HalfWidth;
            float        m_HalfHeight;
            float        m_IsTwoSided;
            unsigned int m_ExposureHistoryIndex;
        };

        struct SRenderJob
        {
            Dt::CAreaLightFacet* m_pDtLightFacet;
            Dt::CEntity*         m_pDtEntity;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CMeshPtr m_QuadModelPtr;

        CBufferPtr m_AreaLightBufferPtr;
        
        CInputLayoutPtr m_QuadInputLayoutPtr;
        CInputLayoutPtr m_LightProbeInputLayoutPtr;
        
        CShaderPtr m_ScreenQuadShaderPtr;
        CShaderPtr m_LTCAreaLightShaderPtr;
        
        CTextureSetPtr m_LTCTextureSetPtr;

        CRenderContextPtr m_DefaultRenderContextPtr;
        CRenderContextPtr m_LightRenderContextPtr;

        CRenderJobs m_RenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxAreaLightRenderer::CGfxAreaLightRenderer()
        : m_QuadModelPtr            ()
        , m_AreaLightBufferPtr      ()
        , m_QuadInputLayoutPtr      ()
        , m_LightProbeInputLayoutPtr()
        , m_ScreenQuadShaderPtr     ()
        , m_LTCAreaLightShaderPtr   ()
        , m_DefaultRenderContextPtr ()
        , m_LightRenderContextPtr   ()
        , m_RenderJobs              ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxAreaLightRenderer::~CGfxAreaLightRenderer()
    {
    
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnExit()
    {
        m_QuadModelPtr             = 0;
        m_AreaLightBufferPtr       = 0;
        m_QuadInputLayoutPtr       = 0;
        m_LightProbeInputLayoutPtr = 0;
        m_ScreenQuadShaderPtr      = 0;
        m_LTCAreaLightShaderPtr    = 0;
        m_LTCTextureSetPtr         = 0;
        m_DefaultRenderContextPtr  = 0;
        m_LightRenderContextPtr    = 0;

        m_RenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupShader()
    {
        m_ScreenQuadShaderPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");

        m_LTCAreaLightShaderPtr = ShaderManager::CompilePS("fs_light_arealight.glsl" , "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_QuadInputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_ScreenQuadShaderPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupRenderTargets()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupStates()
    {
        CCameraPtr          MainCameraPtr      = ViewManager     ::GetMainCamera();

        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();

        CTargetSetPtr       LightTargetSetPtr   = TargetSetManager::GetLightAccumulationTargetSet();
        CTargetSetPtr       DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();

        CRenderStatePtr     DefaultStatePtr = StateManager::GetRenderState(0);
        CRenderStatePtr     LightStatePtr   = StateManager::GetRenderState(CRenderState::AdditionBlend);

        // -----------------------------------------------------------------------------
        
        CRenderContextPtr DefaultContextPtr = ContextManager::CreateRenderContext();
        
        DefaultContextPtr->SetCamera(MainCameraPtr);
        DefaultContextPtr->SetViewPortSet(ViewPortSetPtr);
        DefaultContextPtr->SetTargetSet(DefaultTargetSetPtr);
        DefaultContextPtr->SetRenderState(DefaultStatePtr);
        
        m_DefaultRenderContextPtr = DefaultContextPtr;
        
        CRenderContextPtr LightContextPtr = ContextManager::CreateRenderContext();
        
        LightContextPtr->SetCamera(MainCameraPtr);
        LightContextPtr->SetViewPortSet(ViewPortSetPtr);
        LightContextPtr->SetTargetSet(LightTargetSetPtr);
        LightContextPtr->SetRenderState(LightStatePtr);
        
        m_LightRenderContextPtr = LightContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupTextures()
    {
        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = 64;
        TextureDescriptor.m_NumberOfPixelsV  = 64;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = s_LTCMaterial;
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        CTexture2DPtr LTCMaterialTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 64;
        TextureDescriptor.m_NumberOfPixelsV  = 64;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = s_LTCMag;
        TextureDescriptor.m_Format           = CTextureBase::R16_FLOAT;
        
        CTexture2DPtr LTCMagTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        m_LTCTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(LTCMaterialTexturePtr), static_cast<CTextureBasePtr>(LTCMagTexturePtr));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstantBufferDesc;

        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        ConstantBufferDesc.m_Stride        = 0;
        ConstantBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstantBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SAreaLightProperties);
        ConstantBufferDesc.m_pBytes        = 0;
        ConstantBufferDesc.m_pClassKey     = 0;
        
        m_AreaLightBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::Render()
    {
        Performance::BeginEvent("Area Lights");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob;
        CRenderJobs::const_iterator EndOfRenderJobs;
        const unsigned int          pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_ScreenQuadShaderPtr->GetInputLayout());

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_ScreenQuadShaderPtr);

        ContextManager::SetShaderPS(m_LTCAreaLightShaderPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_AreaLightBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
        ContextManager::SetTexture(4, m_LTCTextureSetPtr->GetTexture(0));
        ContextManager::SetTexture(5, m_LTCTextureSetPtr->GetTexture(1));

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        CurrentRenderJob = m_RenderJobs.begin();
        EndOfRenderJobs  = m_RenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            Dt::CEntity*         pDtEntity     = CurrentRenderJob->m_pDtEntity;
            Dt::CAreaLightFacet* pDtLightFacet = CurrentRenderJob->m_pDtLightFacet;

            assert(pDtEntity && pDtLightFacet);

            Base::Float3 LightPosition  = pDtEntity->GetWorldPosition();
            Base::Float3 LightDirection = pDtLightFacet->GetDirection().Normalize() * Base::Float3(-1.0f);
            Base::Float3 Left           = Base::Float3(0.0f, pDtLightFacet->GetRotation(), 1.0f).Normalize();
            Base::Float3 Right          = LightDirection.CrossProduct(Left).Normalize();

            Left = LightDirection.CrossProduct(Right);

            SAreaLightProperties LightBuffer;

            LightBuffer.m_Color                = Base::Float4(pDtLightFacet->GetLightness(), 1.0f);
            LightBuffer.m_Position             = Base::Float4(LightPosition, 1.0f);
            LightBuffer.m_DirectionX           = Base::Float4(Right, 0.0f);
            LightBuffer.m_DirectionY           = Base::Float4(Left, 0.0f);
            LightBuffer.m_HalfWidth            = 0.5f * pDtLightFacet->GetWidth();
            LightBuffer.m_HalfHeight           = 0.5f * pDtLightFacet->GetHeight();
            LightBuffer.m_Plane                = Base::Float4(LightDirection, -(LightDirection.DotProduct(LightPosition)));
            LightBuffer.m_IsTwoSided           = pDtLightFacet->GetIsTwoSided() ? 1.0f : 0.0f;
            LightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

            BufferManager::UploadConstantBufferData(m_AreaLightBufferPtr, &LightBuffer);

            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        // -----------------------------------------------------------------------------
        // Reset everything
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);
        ContextManager::ResetTexture(4);
        ContextManager::ResetTexture(5);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);
        ContextManager::ResetSampler(4);
        ContextManager::ResetSampler(5);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();
        
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() != Dt::SLightType::Area)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);

                continue;
            }

            SRenderJob NewRenderJob;

            NewRenderJob.m_pDtLightFacet = static_cast<Dt::CAreaLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
            NewRenderJob.m_pDtEntity     = &rCurrentEntity;

            m_RenderJobs.push_back(NewRenderJob);

            // -----------------------------------------------------------------------------
            // Get next light
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }
} // namespace

namespace Gfx
{
namespace LightAreaRenderer
{
    void OnStart()
    {
        CGfxAreaLightRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxAreaLightRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxAreaLightRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxAreaLightRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxAreaLightRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxAreaLightRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxAreaLightRenderer::GetInstance().Render();
    }
} // namespace LightAreaRenderer
} // namespace Gfx
