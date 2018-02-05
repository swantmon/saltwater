
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_map.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "glm.hpp"
#include "ext.hpp"

using namespace Gfx;

namespace
{
    class CGfxParticleRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxParticleRenderer)
        
    public:
        
        CGfxParticleRenderer();
        ~CGfxParticleRenderer();
        
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
        
        enum ELiquidShaderParts
        {
            Depth,
            Thickness,
            GaussianBlur,
            BilateralBlur,
            Shading,
            NumberOfParts,
        };
        
    private:
        
        struct SPerFrameConstantBuffer
        {
            glm::mat4 m_LiquidMatrix;
        };
        
        struct SPerInstanceBuffer
        {
            glm::vec3 m_Offset;
        };
    
        struct SLiquidSettings
        {
            glm::mat4  m_ViewMatrix;
            glm::mat4  m_ProjectionMatrix;
            glm::vec4    m_LightDirection;
            glm::vec4    m_Color;
            float           m_SphereRadius;
        };
        
        struct SBilateralSettings
        {
            glm::vec2 m_Direction;
        };
        
        struct SGaussianSettings
        {
            glm::vec2 m_Direction;
            float        m_Weights[7];
        };
        
        struct SShadingSettings
        {
            glm::mat4 m_InvertedProjectionMatrix;
            glm::mat4 m_InvertedViewMatrix;
            glm::vec4   m_LightDirection;
            glm::vec4   m_ViewDirection;
            glm::vec2   m_InvertedScreensize;
        };
        
        struct SRenderJob
        {
            glm::vec3 m_Position;
            glm::vec3 m_Color;
        };
        
    private:
        
        static const unsigned int s_MaxNumberOfInstances = 512 * 512;

    private:
        
        typedef std::vector<SRenderJob> CParticleEntities;
        
    private:
        
        CMeshPtr          m_ParticleModelPtr;
        CMeshPtr          m_QuadModelPtr;
        
        CBufferSetPtr     m_LiquidVSBufferPtr;
        CBufferSetPtr     m_LiquidPSBufferPtr;
        CBufferSetPtr     m_BilateralPSBufferPtr;
        CBufferSetPtr     m_GaussianPSBufferPtr;
        CBufferSetPtr     m_ShadingPSBufferPtr;
        CBufferPtr        m_ParticleInstanceBufferPtr;
        CBufferPtr        m_ParticleInstanceBufferSetPtr;
        CShaderPtr        m_LiquidShaderVSPtrs[NumberOfParts];
        CShaderPtr        m_LiquidShaderPSPtrs[NumberOfParts];
        CRenderContextPtr m_LiquidContextPtrs[6];
        CTargetSetPtr     m_LiquidTargetSetPtrs[3];
        CTextureSetPtr    m_TextureSetPtrs[4];
        
        CInputLayoutPtr   m_QuadInputLayouPtr;
        CInputLayoutPtr   m_ParticleInputLayouPtr;
        
        CParticleEntities m_Particles;
        float             m_Time;
        
    private:
        
        void BuildRenderJobs();
        
    };
} // namespace

namespace
{
    CGfxParticleRenderer::CGfxParticleRenderer()
        : m_ParticleModelPtr            ()
        , m_QuadModelPtr                ()
        , m_LiquidVSBufferPtr           ()
        , m_LiquidPSBufferPtr           ()
        , m_BilateralPSBufferPtr        ()
        , m_ShadingPSBufferPtr          ()
        , m_ParticleInstanceBufferPtr   ()
        , m_ParticleInstanceBufferSetPtr()
        , m_LiquidContextPtrs           ()
        , m_LiquidShaderVSPtrs          ()
        , m_LiquidShaderPSPtrs          ()
        , m_LiquidTargetSetPtrs         ()
        , m_TextureSetPtrs              ()
        , m_QuadInputLayouPtr           ()
        , m_ParticleInputLayouPtr       ()
        , m_Particles                   ()
        , m_Time                        (0.0f)
    {
        m_Particles.reserve(1024);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxParticleRenderer::~CGfxParticleRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnExit()
    {
        m_ParticleModelPtr             = 0;
        m_QuadModelPtr                 = 0;
        m_LiquidVSBufferPtr            = 0;
        m_LiquidPSBufferPtr            = 0;
        m_BilateralPSBufferPtr         = 0;
        m_GaussianPSBufferPtr          = 0;
        m_ShadingPSBufferPtr           = 0;
        m_ParticleInstanceBufferPtr    = 0;
        m_ParticleInstanceBufferSetPtr = 0;
        m_QuadInputLayouPtr            = 0;
        m_ParticleInputLayouPtr        = 0;
        
        m_LiquidContextPtrs[0] = 0;
        m_LiquidContextPtrs[1] = 0;
        m_LiquidContextPtrs[2] = 0;
        m_LiquidContextPtrs[3] = 0;
        m_LiquidContextPtrs[4] = 0;
        m_LiquidContextPtrs[5] = 0;
        
        m_LiquidTargetSetPtrs[0] = 0;
        m_LiquidTargetSetPtrs[1] = 0;
        m_LiquidTargetSetPtrs[2] = 0;

        m_TextureSetPtrs[0] = 0;
        m_TextureSetPtrs[1] = 0;
        m_TextureSetPtrs[2] = 0;
        m_TextureSetPtrs[3] = 0;
        
        for (unsigned int IndexOfPart = 0; IndexOfPart < NumberOfParts; ++ IndexOfPart)
        {
            m_LiquidShaderVSPtrs[IndexOfPart] = 0;
            m_LiquidShaderPSPtrs[IndexOfPart] = 0;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupShader()
    {
        CShaderPtr ShaderVSPtr;
        CShaderPtr ShaderPSPtr;
        
        ShaderVSPtr = ShaderManager::CompileVS("vs_liquid_particle.glsl", "main");
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_liquid_depth.glsl", "main");
        
        m_LiquidShaderVSPtrs[Depth] = ShaderVSPtr;
        m_LiquidShaderPSPtrs[Depth] = ShaderPSPtr;
        
        // -----------------------------------------------------------------------------
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_liquid_thickness.glsl", "main");
        
        m_LiquidShaderVSPtrs[Thickness] = ShaderVSPtr;
        m_LiquidShaderPSPtrs[Thickness] = ShaderPSPtr;
        
        // -----------------------------------------------------------------------------
        
        ShaderVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_bilateral_blur.glsl", "main");
        
        m_LiquidShaderVSPtrs[BilateralBlur] = ShaderVSPtr;
        m_LiquidShaderPSPtrs[BilateralBlur] = ShaderPSPtr;
        
        // -----------------------------------------------------------------------------
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_gaussian_blur.glsl", "main");
        
        m_LiquidShaderVSPtrs[GaussianBlur] = ShaderVSPtr;
        m_LiquidShaderPSPtrs[GaussianBlur] = ShaderPSPtr;
        
        // -----------------------------------------------------------------------------
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_liquid_shading.glsl", "main");
        
        m_LiquidShaderVSPtrs[Shading] = ShaderVSPtr;
        m_LiquidShaderPSPtrs[Shading] = ShaderPSPtr;
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        const SInputElementDescriptor PositionOffsetInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0,  8, CInputLayout::PerVertex  , 0, },
            { "OFFSET"  , 0, CInputLayout::Float3Format, 1, 0, 12, CInputLayout::PerInstance, 1, },
        };
        
        m_QuadInputLayouPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 1, m_LiquidShaderVSPtrs[BilateralBlur]);
        
        m_ParticleInputLayouPtr = ShaderManager::CreateInputLayout(PositionOffsetInputLayout, 2, m_LiquidShaderVSPtrs[Depth]);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTexture::R32_FLOAT;
        
        CTexturePtr DepthTexturePtr     = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth
        CTexturePtr ThicknessTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Thickness
        CTexturePtr SwapTexturePtr      = TextureManager::CreateTexture2D(RendertargetDescriptor); // Swap
        
        // -----------------------------------------------------------------------------
        // Create liquid buffer target set
        // -----------------------------------------------------------------------------
        CTexturePtr LiquidDepthRenderbufferPtrs[2];
        
        LiquidDepthRenderbufferPtrs[0] = DepthTexturePtr;
        LiquidDepthRenderbufferPtrs[1] = TargetSetManager::GetDefaultTargetSet()->GetDepthStencilTarget();
        
        m_LiquidTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(LiquidDepthRenderbufferPtrs, 2);
        
        // -----------------------------------------------------------------------------
        // Create thickness render target set
        // -----------------------------------------------------------------------------
        CTexturePtr LiquidThicknessRenderbufferPtrs[1];
        
        LiquidThicknessRenderbufferPtrs[0] = ThicknessTexturePtr;
        
        m_LiquidTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(LiquidThicknessRenderbufferPtrs, 1);
        
        // -----------------------------------------------------------------------------
        // Create bilateral filtering target set
        // -----------------------------------------------------------------------------
        CTexturePtr SwapRenderbufferPtrs[1];
        
        SwapRenderbufferPtrs[0] = SwapTexturePtr;
        
        m_LiquidTargetSetPtrs[2] = TargetSetManager::CreateTargetSet(SwapRenderbufferPtrs, 1);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupStates()
    {
        CRenderContextPtr RenderContextPtr;
        
        CViewPortSetPtr ViewPortSetPtr = ViewManager ::GetViewPortSet();

        // -----------------------------------------------------------------------------
        // Depth
        // -----------------------------------------------------------------------------
        RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(m_LiquidTargetSetPtrs[0]);
        RenderContextPtr->SetRenderState(StateManager::GetRenderState(0));
        
        m_LiquidContextPtrs[0] = RenderContextPtr;
        
        // -----------------------------------------------------------------------------
        // Thickness
        // -----------------------------------------------------------------------------
        RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(m_LiquidTargetSetPtrs[1]);
        RenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::AdditionBlend | CRenderState::NoDepth));
        
        m_LiquidContextPtrs[1] = RenderContextPtr;

        // -----------------------------------------------------------------------------
        // Swap
        // -----------------------------------------------------------------------------
        RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(ViewManager::GetFullQuadCamera());
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(m_LiquidTargetSetPtrs[2]);
        RenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoDepth));
        
        m_LiquidContextPtrs[2] = RenderContextPtr;
        
        // -----------------------------------------------------------------------------
        // Depth Blur
        // -----------------------------------------------------------------------------
        RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(ViewManager::GetFullQuadCamera());
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(m_LiquidTargetSetPtrs[0]);
        RenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoDepth));
        
        m_LiquidContextPtrs[3] = RenderContextPtr;
        
        // -----------------------------------------------------------------------------
        // Thickness Blur
        // -----------------------------------------------------------------------------
        RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(ViewManager::GetFullQuadCamera());
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(m_LiquidTargetSetPtrs[1]);
        RenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoDepth));
        
        m_LiquidContextPtrs[4] = RenderContextPtr;
        
        // -----------------------------------------------------------------------------
        // Shading
        // -----------------------------------------------------------------------------
        RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(ViewManager::GetFullQuadCamera());
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(TargetSetManager::GetDefaultTargetSet());
        RenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoDepth | CRenderState::AlphaBlend));
        
        m_LiquidContextPtrs[5] = RenderContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupTextures()
    {
        m_TextureSetPtrs[0] = TextureManager::CreateTextureSet(m_LiquidTargetSetPtrs[0]->GetRenderTarget(0));
        m_TextureSetPtrs[1] = TextureManager::CreateTextureSet(m_LiquidTargetSetPtrs[1]->GetRenderTarget(0));
        
        m_TextureSetPtrs[2] = TextureManager::CreateTextureSet(m_LiquidTargetSetPtrs[2]->GetRenderTarget(0));
        
        m_TextureSetPtrs[3] = TextureManager::CreateTextureSet(/* static_cast<CTextureBasePtr>(AtmosphereRenderer::GetCubemap()), */ TargetSetManager::GetDefaultTargetSet()->GetRenderTarget(0), m_LiquidTargetSetPtrs[0]->GetRenderTarget(0), m_LiquidTargetSetPtrs[1]->GetRenderTarget(0));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupBuffers()
    {
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerFrameConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerFrameConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SLiquidSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr LiquidSettingsBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SBilateralSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr BilateralSettingsBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GaussianSettingsBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SShadingSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ShadingSettingsBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerInstanceBuffer) * s_MaxNumberOfInstances;
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_ParticleInstanceBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        m_LiquidVSBufferPtr         = BufferManager::CreateBufferSet(PerFrameConstantBuffer);
               
        m_LiquidPSBufferPtr         = BufferManager::CreateBufferSet(LiquidSettingsBuffer);
        
        m_BilateralPSBufferPtr      = BufferManager::CreateBufferSet(BilateralSettingsBuffer);
        
        m_GaussianPSBufferPtr       = BufferManager::CreateBufferSet(GaussianSettingsBuffer);
        
        m_ShadingPSBufferPtr        = BufferManager::CreateBufferSet(ShadingSettingsBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupModels()
    {
        m_ParticleModelPtr = MeshManager::CreateRectangle(-0.5f, -0.5f, 1.0f, 1.0f);
        
        m_QuadModelPtr     = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::Update()
    {
        // -----------------------------------------------------------------------------
        // Upload some frame data to graphic device
        // -----------------------------------------------------------------------------
        SPerFrameConstantBuffer PerFrameConstantBuffer;
        
        PerFrameConstantBuffer.m_LiquidMatrix  = glm::mat4(1.0f);
        PerFrameConstantBuffer.m_LiquidMatrix *= glm::transpose(glm::mat4(ViewManager::GetMainCamera()->GetView()->GetRotationMatrix()));
        PerFrameConstantBuffer.m_LiquidMatrix *= glm::eulerAngleX(Base::DegreesToRadians(-180.0f));
        PerFrameConstantBuffer.m_LiquidMatrix *= glm::scale(glm::vec3(0.3f));
        
        BufferManager::UploadBufferData(m_LiquidVSBufferPtr->GetBuffer(0), &PerFrameConstantBuffer);
        
        // -----------------------------------------------------------------------------
        // Clear render targets
        // -----------------------------------------------------------------------------
        TargetSetManager::ClearTargetSet(m_LiquidTargetSetPtrs[0], glm::vec4(1.0f));
        TargetSetManager::ClearTargetSet(m_LiquidTargetSetPtrs[1], glm::vec4(0.0f));
        TargetSetManager::ClearTargetSet(m_LiquidTargetSetPtrs[2]);
        
        // -----------------------------------------------------------------------------
        // Build render jobs
        // -----------------------------------------------------------------------------
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::Render()
    {
        // -----------------------------------------------------------------------------
        // Screen-Space liquid spheres (depth)
        // -----------------------------------------------------------------------------
        CParticleEntities::const_iterator EndOfParticles;

        ContextManager::SetRenderContext(m_LiquidContextPtrs[0]);

        ContextManager::SetVertexBuffer(m_ParticleInstanceBufferSetPtr);

        ContextManager::SetIndexBuffer(m_ParticleModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_ParticleInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[Depth]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[Depth]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_LiquidVSBufferPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(2, m_LiquidPSBufferPtr->GetBuffer(0));

        SLiquidSettings LiquidBuffer;

        LiquidBuffer.m_ViewMatrix = ViewManager::GetMainCamera()->GetView()->GetViewMatrix();
        LiquidBuffer.m_ProjectionMatrix = ViewManager::GetMainCamera()->GetProjectionMatrix();
        LiquidBuffer.m_LightDirection = glm::vec4(-0.4f, -0.3f, -1.0f, 0.0f);
        LiquidBuffer.m_Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        LiquidBuffer.m_SphereRadius = 0.3f;

        BufferManager::UploadBufferData(m_LiquidPSBufferPtr->GetBuffer(0), &LiquidBuffer);

        // TODO by tschwandt
        // Use a global buffer with instance ID instead of multiple vertex buffer objects
//         pInstances = BufferManager::MapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1), CBuffer::Write);
// 
//         assert(pInstances != 0);
// 
//         EndOfParticles = m_Particles.end();
// 
//         for (CParticleEntities::const_iterator CurrentParticle = m_Particles.begin(); CurrentParticle != EndOfParticles; ++CurrentParticle)
//         {
//             pInstance = &(static_cast<SPerInstanceBuffer*>(pInstances))[NumberOfParticles++];
// 
//             pInstance->m_Offset = CurrentParticle->m_Position;
// 
//             if (NumberOfParticles == s_MaxNumberOfInstances)
//             {
//                 BufferManager::UnmapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1));
// 
//                 ContextManager::DrawIndexedInstanced(m_ParticleModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), NumberOfParticles, 0, 0, 0);
// 
//                 pInstances = BufferManager::MapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1), CBuffer::Write);
// 
//                 NumberOfParticles = 0;
//             }
//         }
// 
//         if (NumberOfParticles > 0)
//         {
//             BufferManager::UnmapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1));
// 
//             ContextManager::DrawIndexedInstanced(m_ParticleModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), NumberOfParticles, 0, 0, 0);
// 
//             NumberOfParticles = 0;
//         }

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Screen-Space liquid spheres (thickness) with same buffers and same number
        // of particles.
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LiquidContextPtrs[1]);

        ContextManager::SetVertexBuffer(m_ParticleInstanceBufferSetPtr);

        ContextManager::SetIndexBuffer(m_ParticleModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_ParticleInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[Thickness]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[Thickness]);

        ContextManager::SetConstantBuffer(0, m_LiquidVSBufferPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(1, m_LiquidVSBufferPtr->GetBuffer(1));

//         pInstances = BufferManager::MapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1), CBuffer::Write);
// 
//         assert(pInstances != 0);
// 
//         EndOfParticles = m_Particles.end();
// 
//         for (CParticleEntities::const_iterator CurrentParticle = m_Particles.begin(); CurrentParticle != EndOfParticles; ++CurrentParticle)
//         {
//             pInstance = &(static_cast<SPerInstanceBuffer*>(pInstances))[NumberOfParticles++];
// 
//             pInstance->m_Offset = CurrentParticle->m_Position;
// 
//             if (NumberOfParticles == s_MaxNumberOfInstances)
//             {
//                 BufferManager::UnmapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1));
// 
//                 ContextManager::DrawIndexedInstanced(m_ParticleModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), NumberOfParticles, 0, 0, 0);
// 
//                 pInstances = BufferManager::MapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1), CBuffer::Write);
// 
//                 NumberOfParticles = 0;
//             }
//         }
// 
//         if (NumberOfParticles > 0)
//         {
//             BufferManager::UnmapVertexBuffer(m_ParticleInstanceBufferSetPtr->GetBuffer(1));
// 
//             ContextManager::DrawIndexedInstanced(m_ParticleModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), NumberOfParticles, 0, 0, 0);
// 
//             NumberOfParticles = 0;
//         }

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Bilateral Filter
        // -----------------------------------------------------------------------------
        SBilateralSettings BilateralSettings;

        ContextManager::SetRenderContext(m_LiquidContextPtrs[2]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[BilateralBlur]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[BilateralBlur]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_BilateralPSBufferPtr->GetBuffer(0));

        BilateralSettings.m_Direction[0] = 1.0f * 1.0f / static_cast<float>(1280.0f);
        BilateralSettings.m_Direction[1] = 0.0f * 1.0f / static_cast<float>(800.0f);

        BufferManager::UploadBufferData(m_BilateralPSBufferPtr->GetBuffer(0), &BilateralSettings);

        ContextManager::SetTexture(0, m_TextureSetPtrs[0]->GetTexture(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------

        ContextManager::SetRenderContext(m_LiquidContextPtrs[3]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[BilateralBlur]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[BilateralBlur]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_BilateralPSBufferPtr->GetBuffer(0));

        BilateralSettings.m_Direction[0] = 0.0f * 1.0f / static_cast<float>(1280.0f);
        BilateralSettings.m_Direction[1] = 1.0f * 1.0f / static_cast<float>(800.0f);

        BufferManager::UploadBufferData(m_BilateralPSBufferPtr->GetBuffer(0), &BilateralSettings);

        ContextManager::SetTexture(0, m_TextureSetPtrs[2]->GetTexture(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Rendering: Do gaussian blur
        // -----------------------------------------------------------------------------
        SGaussianSettings GaussianSettings;

        GaussianSettings.m_Weights[0] = 0.064493f;
        GaussianSettings.m_Weights[1] = 0.070273f;
        GaussianSettings.m_Weights[2] = 0.075385f;
        GaussianSettings.m_Weights[3] = 0.079617f;
        GaussianSettings.m_Weights[4] = 0.082784f;
        GaussianSettings.m_Weights[5] = 0.084745f;
        GaussianSettings.m_Weights[6] = 0.085408f;

        ContextManager::SetRenderContext(m_LiquidContextPtrs[2]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[GaussianBlur]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[GaussianBlur]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_GaussianPSBufferPtr->GetBuffer(0));

        GaussianSettings.m_Direction[0] = 1.0f * 1.0f / static_cast<float>(1280);
        GaussianSettings.m_Direction[1] = 0.0f * 1.0f / static_cast<float>(800);

        BufferManager::UploadBufferData(m_GaussianPSBufferPtr->GetBuffer(0), &GaussianSettings);

        ContextManager::SetTexture(0, m_TextureSetPtrs[1]->GetTexture(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------

        ContextManager::SetRenderContext(m_LiquidContextPtrs[4]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[GaussianBlur]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[GaussianBlur]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_GaussianPSBufferPtr->GetBuffer(0));

        GaussianSettings.m_Direction[0] = 0.0f * 1.0f / static_cast<float>(1280);
        GaussianSettings.m_Direction[1] = 1.0f * 1.0f / static_cast<float>(800);

        BufferManager::UploadBufferData(m_GaussianPSBufferPtr->GetBuffer(0), &GaussianSettings);

        ContextManager::SetTexture(0, m_TextureSetPtrs[2]->GetTexture(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Final water shading
        // -----------------------------------------------------------------------------
        SShadingSettings ShadingSettings;

        ShadingSettings.m_InvertedProjectionMatrix = glm::inverse(ViewManager::GetMainCamera()->GetProjectionMatrix());
        ShadingSettings.m_InvertedViewMatrix       = glm::inverse(ViewManager::GetMainCamera()->GetView()->GetViewMatrix());
        ShadingSettings.m_LightDirection           = glm::vec4(-0.4f, -0.3f, -1.0f, 0.0f);
        ShadingSettings.m_ViewDirection            = glm::vec4(ViewManager::GetMainCamera()->GetView()->GetViewDirection(), 0.0f);
        ShadingSettings.m_InvertedScreensize[0]    = 1.0f / static_cast<float>(1280.0f);
        ShadingSettings.m_InvertedScreensize[1]    = 1.0f / static_cast<float>(800.0f);

        BufferManager::UploadBufferData(m_ShadingPSBufferPtr->GetBuffer(0), &ShadingSettings);

        ContextManager::SetRenderContext(m_LiquidContextPtrs[5]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayouPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_LiquidShaderVSPtrs[Shading]);

        ContextManager::SetShaderPS(m_LiquidShaderPSPtrs[Shading]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_ShadingPSBufferPtr->GetBuffer(0));

        ContextManager::SetTexture(0, m_TextureSetPtrs[3]->GetTexture(0));
        ContextManager::SetTexture(1, m_TextureSetPtrs[3]->GetTexture(1));
        ContextManager::SetTexture(2, m_TextureSetPtrs[3]->GetTexture(2));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);

        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);

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
    
    // -----------------------------------------------------------------------------
    
    void CGfxParticleRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_Particles.clear();
        
        // -----------------------------------------------------------------------------
        // Iterate throw every particle system inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();
        
        for (; CurrentEntity != EndOfEntities; )
        {
            
            
            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
        }
    }
} // namespace


namespace Gfx
{
namespace ParticleRenderer
{
    void OnStart()
    {
        CGfxParticleRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxParticleRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxParticleRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxParticleRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxParticleRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxParticleRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxParticleRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxParticleRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxParticleRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxParticleRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxParticleRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxParticleRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxParticleRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxParticleRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxParticleRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxParticleRenderer::GetInstance().Render();
    }
} // namespace ParticleRenderer
} // namespace Gfx

