
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_map.h"
#include "engine/data/data_point_light_component.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_histogram_renderer.h"
#include "engine/graphic/gfx_light_indirect_renderer.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_point_light.h"
#include "engine/graphic/gfx_point_light_manager.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxLightIndirectRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLightIndirectRenderer)
        
    public:
        CGfxLightIndirectRenderer();
        ~CGfxLightIndirectRenderer();
        
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

        void ResetSettings();
        void SetSettings(const SIndirectLightSettings& _rSettings);
        const SIndirectLightSettings& GetSettings();
        
    private:

        struct SIndirectLightProperties
        {
            glm::vec4 m_RSMSettings;
            unsigned int m_ExposureHistoryIndex;
        };

        struct SRenderJob
        {
            Gfx::CPointLight* m_pGraphicPointLightFacet;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CBufferPtr  m_IndirectLightPSBufferPtr;
        CShaderPtr  m_RectangleShaderVSPtr;
        CShaderPtr  m_IndirectLightShaderPSPtr;
        CRenderJobs m_RenderJobs;

        SIndirectLightSettings m_IndirectLightSettings;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxLightIndirectRenderer::CGfxLightIndirectRenderer()
        : m_IndirectLightPSBufferPtr()
        , m_IndirectLightShaderPSPtr()
        , m_RectangleShaderVSPtr    ()
        , m_RenderJobs              ()
    {
        m_RenderJobs.reserve(4);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxLightIndirectRenderer::~CGfxLightIndirectRenderer()
    {
    	
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnExit()
    {
        m_IndirectLightPSBufferPtr = 0;
        m_IndirectLightShaderPSPtr = 0;
        m_RectangleShaderVSPtr     = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupShader()
    {       
        m_RectangleShaderVSPtr = ShaderManager::CompileVS("system/vs_fullscreen.glsl", "main");
        
        m_IndirectLightShaderPSPtr = ShaderManager::CompilePS("indirect_light/fs_light_indirectlight.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupStates()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SIndirectLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_IndirectLightPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupModels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::Render()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("Indirect Lights");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AdditionBlend));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_IndirectLightShaderPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_IndirectLightPSBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(6, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(7, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        // -----------------------------------------------------------------------------
        // Set textures
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        Gfx::ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        Gfx::ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        Gfx::ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

        // -----------------------------------------------------------------------------
        // Iterate throw every light
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin();
        CRenderJobs::const_iterator EndOfRenderJobs  = m_RenderJobs.end();

        for (auto& rCurrentRenderJob : m_RenderJobs)
        {
            Gfx::CPointLight* pGfxPointLight = rCurrentRenderJob.m_pGraphicPointLightFacet;

            assert(pGfxPointLight != nullptr);

            // -----------------------------------------------------------------------------
            // Set shadow map
            // -----------------------------------------------------------------------------
            Gfx::ContextManager::SetTexture(4, pGfxPointLight->GetTextureRSMSet()->GetTexture(0));
            Gfx::ContextManager::SetTexture(5, pGfxPointLight->GetTextureRSMSet()->GetTexture(1));
            Gfx::ContextManager::SetTexture(6, pGfxPointLight->GetTextureRSMSet()->GetTexture(2));
            Gfx::ContextManager::SetTexture(7, pGfxPointLight->GetTextureRSMSet()->GetTexture(3));

            int HeightOfShadowmap = static_cast<int>(pGfxPointLight->GetShadowmapSize()) / static_cast<int>(m_IndirectLightSettings.m_RSMSplitting);

            for (int IndexOfRSMDataY = 0; IndexOfRSMDataY < HeightOfShadowmap; ++IndexOfRSMDataY)
            {
                // -----------------------------------------------------------------------------
                // Upload buffer data
                // -----------------------------------------------------------------------------
                SIndirectLightProperties IndirectLightBuffer;

                float SSWidthOfShadowmap = static_cast<float>(HeightOfShadowmap);

                IndirectLightBuffer.m_RSMSettings[0] = 1.0f / SSWidthOfShadowmap;
                IndirectLightBuffer.m_RSMSettings[1] = static_cast<float>(IndexOfRSMDataY) * 1.0f / SSWidthOfShadowmap;
                IndirectLightBuffer.m_RSMSettings[2] = SSWidthOfShadowmap;
                IndirectLightBuffer.m_RSMSettings[3] = 0.0f;
                IndirectLightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

                Gfx::BufferManager::UploadBufferData(m_IndirectLightPSBufferPtr, &IndirectLightBuffer);

                // -----------------------------------------------------------------------------
                // Draw
                // -----------------------------------------------------------------------------
                Gfx::ContextManager::Draw(3, 0);
            }
        }

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::ResetTexture(0);
        Gfx::ContextManager::ResetTexture(1);
        Gfx::ContextManager::ResetTexture(2);
        Gfx::ContextManager::ResetTexture(3);
        Gfx::ContextManager::ResetTexture(4);
        Gfx::ContextManager::ResetTexture(5);
        Gfx::ContextManager::ResetTexture(6);
        Gfx::ContextManager::ResetTexture(7);

        ContextManager::ResetConstantBuffer(0); 

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetResourceBuffer(0);

        Gfx::ContextManager::ResetSampler(0);
        Gfx::ContextManager::ResetSampler(1);
        Gfx::ContextManager::ResetSampler(2);
        Gfx::ContextManager::ResetSampler(3);
        Gfx::ContextManager::ResetSampler(4);
        Gfx::ContextManager::ResetSampler(5);
        Gfx::ContextManager::ResetSampler(6);
        Gfx::ContextManager::ResetSampler(7);

        ContextManager::ResetTopology();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightIndirectRenderer::ResetSettings()
    {
        SIndirectLightSettings Settings;

        Settings.m_RSMSplitting = Core::CProgramParameters::GetInstance().Get("graphics:indirect_light:RSM_splitting", 8.0f);

        SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightIndirectRenderer::SetSettings(const SIndirectLightSettings& _rSettings)
    {
        m_IndirectLightSettings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    const SIndirectLightSettings& CGfxLightIndirectRenderer::GetSettings()
    {
        return m_IndirectLightSettings;
    }

    // -----------------------------------------------------------------------------

    void CGfxLightIndirectRenderer::BuildRenderJobs()
    {
        m_RenderJobs.clear();

        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CPointLightComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CPointLightComponent*  pDtComponent  = static_cast<Dt::CPointLightComponent*>(Component);

            Gfx::CPointLight* pGfxComponent =static_cast<Gfx::CPointLight*>(pDtComponent->GetFacet(Dt::CPointLightComponent::Graphic));

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            if (pDtComponent->GetShadowType() == Dt::CPointLightComponent::GlobalIllumination)
            {
                SRenderJob NewRenderJob;

                NewRenderJob.m_pGraphicPointLightFacet = pGfxComponent;

                m_RenderJobs.push_back(NewRenderJob);
            }
        }
    }
} // namespace

namespace Gfx
{
namespace LightIndirectRenderer
{
    void OnStart()
    {
        CGfxLightIndirectRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxLightIndirectRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxLightIndirectRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxLightIndirectRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxLightIndirectRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxLightIndirectRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxLightIndirectRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxLightIndirectRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SIndirectLightSettings& _rSettings)
    {
        CGfxLightIndirectRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SIndirectLightSettings& GetSettings()
    {
        return CGfxLightIndirectRenderer::GetInstance().GetSettings();
    }
} // namespace LightIndirectRenderer
} // namespace Gfx

