
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_area_light_component.h"
#include "data/data_component_facet.h"
#include "data/data_component.h"
#include "data/data_entity.h"
#include "data/data_map.h"

#include "graphic/gfx_area_light_component.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_component.h"
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
        void RenderBulbs();
        
    private:
                
        struct SAreaLightProperties
        {
            glm::vec4    m_Color;
            glm::vec4    m_Position;
            glm::vec4    m_DirectionX;
            glm::vec4    m_DirectionY;
            glm::vec4    m_Plane;
            float        m_HalfWidth;
            float        m_HalfHeight;
            float        m_IsTwoSided;
            float        m_IsTextured;
            unsigned int m_ExposureHistoryIndex;
        };

        struct SAreaLightbulbProperties
        {
            glm::vec4 m_Color;
        };

        struct SRenderJob
        {
            Gfx::CAreaLightComponent* m_pGfxComponent;
            Dt::CAreaLightComponent*  m_pDtComponent;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CMeshPtr m_QuadModelPtr;

        CBufferPtr m_AreaLightBufferPtr;
        CBufferPtr m_AreaLightbulbBufferPtr;
        
        CInputLayoutPtr m_P2InputLayoutPtr;
        CInputLayoutPtr m_P3T2InputLayoutPtr;
        
        CShaderPtr m_PositionShaderPtr;
        CShaderPtr m_ScreenQuadShaderPtr;
        CShaderPtr m_LTCAreaLightShaderPtr;
        CShaderPtr m_AreaLightbulbShaderPtr;
        
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
        , m_AreaLightbulbBufferPtr  ()
        , m_P2InputLayoutPtr        ()
        , m_P3T2InputLayoutPtr      ()
        , m_PositionShaderPtr       ()
        , m_ScreenQuadShaderPtr     ()
        , m_LTCAreaLightShaderPtr   ()
        , m_AreaLightbulbShaderPtr  ()
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
        m_AreaLightbulbBufferPtr   = 0;
        m_P2InputLayoutPtr         = 0;
        m_P3T2InputLayoutPtr       = 0;
        m_PositionShaderPtr        = 0;
        m_ScreenQuadShaderPtr      = 0;
        m_LTCAreaLightShaderPtr    = 0;
        m_AreaLightbulbShaderPtr   = 0;
        m_LTCTextureSetPtr         = 0;
        m_DefaultRenderContextPtr  = 0;
        m_LightRenderContextPtr    = 0;

        m_RenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupShader()
    {
        m_PositionShaderPtr = ShaderManager::CompileVS("vs_p3t2.glsl", "main");

        m_ScreenQuadShaderPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");

        m_LTCAreaLightShaderPtr = ShaderManager::CompilePS("fs_light_arealight.glsl" , "main");

        m_AreaLightbulbShaderPtr = ShaderManager::CompilePS("fs_light_arealight_bulb.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor P2InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        const SInputElementDescriptor P3T2InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 20, CInputLayout::PerVertex, 0, },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 12, 20, CInputLayout::PerVertex, 0, },
        };
        
        m_P2InputLayoutPtr = ShaderManager::CreateInputLayout(P2InputLayout, 1, m_ScreenQuadShaderPtr);

        m_P3T2InputLayoutPtr = ShaderManager::CreateInputLayout(P3T2InputLayout, 2, m_PositionShaderPtr);
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

        CRenderStatePtr     DefaultStatePtr = StateManager::GetRenderState(CRenderState::NoCull);
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
        
        TextureDescriptor.m_NumberOfPixelsU  = 32;
        TextureDescriptor.m_NumberOfPixelsV  = 32;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::Unknown;
        TextureDescriptor.m_Usage            = CTexture::GPURead;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = s_LTCMaterial;
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        CTexturePtr LTCMaterialTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

		TextureManager::SetTextureLabel(LTCMaterialTexturePtr, "Area light LTC Material Texture");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 32;
        TextureDescriptor.m_NumberOfPixelsV  = 32;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::Unknown;
        TextureDescriptor.m_Usage            = CTexture::GPURead;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = s_LTCMag;
        TextureDescriptor.m_Format           = CTexture::R16G16_FLOAT;
        
        CTexturePtr LTCMagTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

		TextureManager::SetTextureLabel(LTCMagTexturePtr, "Area light LTC Mag Texture");

        // -----------------------------------------------------------------------------

        m_LTCTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(LTCMaterialTexturePtr), static_cast<CTexturePtr>(LTCMagTexturePtr));
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

        // -----------------------------------------------------------------------------

        ConstantBufferDesc.m_Stride        = 0;
        ConstantBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstantBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SAreaLightbulbProperties);
        ConstantBufferDesc.m_pBytes        = 0;
        ConstantBufferDesc.m_pClassKey     = 0;

        m_AreaLightbulbBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
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
        ShaderManager::ReloadShader(m_LTCAreaLightShaderPtr);
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
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("Area Lights");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob;
        CRenderJobs::const_iterator EndOfRenderJobs;

        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_ScreenQuadShaderPtr->GetInputLayout());

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_ScreenQuadShaderPtr);

        ContextManager::SetShaderPS(m_LTCAreaLightShaderPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

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
            Dt::CAreaLightComponent*  pDtComponent  = CurrentRenderJob->m_pDtComponent;
            Gfx::CAreaLightComponent* pGfxComponent = CurrentRenderJob->m_pGfxComponent;

            assert(pDtComponent && pGfxComponent);

            // -----------------------------------------------------------------------------
            // Update data
            // -----------------------------------------------------------------------------
            SAreaLightProperties LightBuffer;

            LightBuffer.m_Color                = glm::vec4(pDtComponent->GetLightness(), pDtComponent->GetIntensity());
            LightBuffer.m_Position             = glm::vec4(pDtComponent->GetHostEntity()->GetWorldPosition(), 1.0f);
            LightBuffer.m_DirectionX           = pGfxComponent->GetDirectionX();
            LightBuffer.m_DirectionY           = pGfxComponent->GetDirectionY();
            LightBuffer.m_HalfWidth            = pGfxComponent->GetHalfWidth();
            LightBuffer.m_HalfHeight           = pGfxComponent->GetHalfHeight();
            LightBuffer.m_Plane                = pGfxComponent->GetPlane();
            LightBuffer.m_IsTwoSided           = pDtComponent->GetIsTwoSided() ? 1.0f : 0.0f;
            LightBuffer.m_IsTextured           = pGfxComponent->HasTexture() ? 1.0f : 0.0f;
            LightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

            BufferManager::UploadBufferData(m_AreaLightBufferPtr, &LightBuffer);

            // -----------------------------------------------------------------------------
            // Set texture
            // -----------------------------------------------------------------------------
            if (pGfxComponent->HasTexture())
            {
                ContextManager::SetSampler(6, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

                ContextManager::SetTexture(6, static_cast<Gfx::CTexturePtr>(pGfxComponent->GetFilteredTexturePtr()));
            }

            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);
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

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();
        
        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::RenderBulbs()
    {
        Performance::BeginEvent("Area Lights Bulbs");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob;
        CRenderJobs::const_iterator EndOfRenderJobs;

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        CurrentRenderJob = m_RenderJobs.begin();
        EndOfRenderJobs  = m_RenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            Dt::CAreaLightComponent*  pDtComponent  = CurrentRenderJob->m_pDtComponent;
            Gfx::CAreaLightComponent* pGfxComponent = CurrentRenderJob->m_pGfxComponent;

            assert(pDtComponent && pGfxComponent);

            ContextManager::SetRenderContext(m_DefaultRenderContextPtr);

            ContextManager::SetVertexBuffer(pGfxComponent->GetPlaneVertexBuffer());

            ContextManager::SetIndexBuffer(pGfxComponent->GetPlaneIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_PositionShaderPtr->GetInputLayout());

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_PositionShaderPtr);

            ContextManager::SetShaderPS(m_AreaLightbulbShaderPtr);

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
            ContextManager::SetConstantBuffer(1, m_AreaLightbulbBufferPtr);

            // -----------------------------------------------------------------------------

            SAreaLightbulbProperties LightBuffer;

            LightBuffer.m_Color = glm::vec4(pDtComponent->GetColor(), pGfxComponent->HasTexture() ? 1.0f : 0.0f);

            BufferManager::UploadBufferData(m_AreaLightbulbBufferPtr, &LightBuffer);

            // -----------------------------------------------------------------------------

            if (pGfxComponent->HasTexture())
            {
                ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

                ContextManager::SetTexture(0, static_cast<Gfx::CTexturePtr>(pGfxComponent->GetTexturePtr()));
            }

            // -----------------------------------------------------------------------------

            ContextManager::DrawIndexed(6, 0, 0);

            // -----------------------------------------------------------------------------
            // Reset everything
            // -----------------------------------------------------------------------------
            ContextManager::ResetConstantBuffer(0);
            ContextManager::ResetConstantBuffer(1);

            ContextManager::ResetTopology();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBuffer();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetRenderContext();
        }

        
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightRenderer::BuildRenderJobs()
    {
        m_RenderJobs.clear();

        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CAreaLightComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CAreaLightComponent* pDtComponent = static_cast<Dt::CAreaLightComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            SRenderJob NewRenderJob;

            NewRenderJob.m_pDtComponent  = pDtComponent;
            NewRenderJob.m_pGfxComponent = Gfx::CComponentManager::GetInstance().GetComponent<Gfx::CAreaLightComponent>(NewRenderJob.m_pDtComponent->GetID());

            m_RenderJobs.push_back(NewRenderJob);
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

    // -----------------------------------------------------------------------------

    void RenderBulbs()
    {
        CGfxAreaLightRenderer::GetInstance().RenderBulbs();
    }
} // namespace LightAreaRenderer
} // namespace Gfx
