
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

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
        
        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };
                
        struct SAreaLightProperties
        {
            Base::Float4   dcolor;                                  //> Diffuse Color {r:1.0, g:1.0, b:1.0}
            Base::Float4   scolor;                                  //> Specular Color {r:1.0, g:1.0, b:1.0}
            float          intensity;                               //> Light Intensity {default:4, min:0, max:10}
            float          width;                                   //> Width {default: 8, min:0.1, max:15, step:0.1}
            float          height;                                  //> Height {default: 8, min:0.1, max:15, step:0.1}
            float          rotz;                                    //> Rotation Z {default: 0, min:0, max:1, step:0.001}
            bool           twoSided;                                //> Two-sided {default:false}
            unsigned int   m_ExposureHistoryIndex;
        };
        
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
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerDrawCallConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SAreaLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_AreaLightBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
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
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::Render()
    {
        Performance::BeginEvent("Area Lights");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

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
        SAreaLightProperties LightBuffer;

        LightBuffer.dcolor                 = Base::Float4(1.0f);
        LightBuffer.scolor                 = Base::Float4(1.0f);
        LightBuffer.intensity              = 100000.0f;
        LightBuffer.width                  = 8.0f;
        LightBuffer.height                 = 8.0f;
        LightBuffer.rotz                   = 0.123f;
        LightBuffer.twoSided               = false;
        LightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

        BufferManager::UploadConstantBufferData(m_AreaLightBufferPtr, &LightBuffer);

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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
