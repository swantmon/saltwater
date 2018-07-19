
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_light_probe_component.h"
#include "engine/data/data_map.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_point_light_component.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_debug.h"
#include "engine/graphic/gfx_histogram_renderer.h"
#include "engine/graphic/gfx_light_probe.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_material.h"
#include "engine/graphic/gfx_material_manager.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_mesh_renderer.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_point_light.h"
#include "engine/graphic/gfx_reflection_renderer.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sun.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include <algorithm>
#include <vector>

using namespace Gfx;

namespace
{
    class CGfxRefractionRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxRefractionRenderer)

    public:

        CGfxRefractionRenderer();
        ~CGfxRefractionRenderer();

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

        void OnResize(unsigned int _Width, unsigned int _Height);

        void Update();
        void RenderForward();

    private:

        static const unsigned int s_MaxNumberOfLights = 4;

    private:

        struct SPerDrawCallConstantBufferVS
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SLightProperties
        {
            glm::mat4    m_LightViewProjection;
            glm::vec4    m_LightPosition;
            glm::vec4    m_LightDirection;
            glm::vec4    m_LightColor;
            glm::vec4    m_LightSettings;
            unsigned int m_LightType;
            unsigned int m_Padding0;
            unsigned int m_Padding1;
            unsigned int m_Padding2;
        };

        struct SForwardPassProperties
        {
            glm::vec4    m_CameraPosition;
            unsigned int m_ExposureHistoryIndex;
        };

        struct SCausticSettingsBuffer
        {
            glm::vec4 m_RefractionIndices;
            glm::vec4 m_DepthLinearization;
        };

        struct SRenderJob
        {
            unsigned int     m_SurfaceAttributes;
            Base::ID         m_EntityID;
            CSurfacePtr      m_SurfacePtr;
            const CMaterial* m_SurfaceMaterialPtr;
            glm::mat4        m_ModelMatrix;
        };

        struct SLightJob
        {
            CTexturePtr m_ShadowTexturePtrs[s_MaxNumberOfLights];
            CTexturePtr m_SpecularTexturePtr;
            CTexturePtr m_DiffuseTexturePtr;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        typedef std::vector<SLightJob> CLightJobs;

    private:

        CTexturePtr       m_RefractiveNormalTexturePtr;
        CTexturePtr       m_RefractiveDepthTexturePtr;
        CShaderPtr        m_FullscreenVSPtr;
        CShaderPtr        m_NormalPSPtr;
        CShaderPtr        m_NormalTexPSPtr;
        CShaderPtr        m_RefractionApplyPSPtr;
        CTargetSetPtr     m_RefractionTargetSetPtr;
        CBufferPtr        m_CausticSettingsBufferPtr;

        CBufferPtr        m_ModelBufferPtr;
        CBufferPtr        m_SurfaceMaterialBufferPtr;
        CBufferPtr        m_HitProxyPassPSBufferPtr;
        CBufferPtr        m_ForwardPassBufferPtr;
        CBufferPtr        m_LightPropertiesBufferPtr;
        CShaderPtr        m_HitProxyShaderPtr;
        CRenderContextPtr m_DeferredContextPtr;
        CRenderContextPtr m_HitProxyContextPtr;
        CRenderJobs       m_RefractionRenderJobs;
        SLightJob         m_ForwardLightTextures;

    private:

        void BuildRenderJobs();
        void UpdateLightProperties();
    };
} // namespace

namespace
{
    CGfxRefractionRenderer::CGfxRefractionRenderer()
        : m_ModelBufferPtr          ()
        , m_SurfaceMaterialBufferPtr()
        , m_HitProxyPassPSBufferPtr ()
        , m_ForwardPassBufferPtr    ()
        , m_LightPropertiesBufferPtr()
        , m_HitProxyShaderPtr       ()
        , m_DeferredContextPtr      ()
        , m_HitProxyContextPtr      ()
        , m_ForwardLightTextures    ()
    {
        // -----------------------------------------------------------------------------
        // Reserve some jobs
        // -----------------------------------------------------------------------------
        m_RefractionRenderJobs.reserve(4);

        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxRefractionRenderer::OnResize));
    }

    // -----------------------------------------------------------------------------

    CGfxRefractionRenderer::~CGfxRefractionRenderer()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnExit()
    {
        m_RefractiveNormalTexturePtr = 0;
        m_RefractiveDepthTexturePtr = 0;
        m_FullscreenVSPtr = 0;
        m_NormalPSPtr = 0;
        m_NormalTexPSPtr = 0;
        m_RefractionApplyPSPtr = 0;
        m_RefractionTargetSetPtr = 0;
        m_CausticSettingsBufferPtr = 0;

        m_ModelBufferPtr           = 0;
        m_SurfaceMaterialBufferPtr = 0;
        m_HitProxyPassPSBufferPtr  = 0;
        m_ForwardPassBufferPtr     = 0;
        m_LightPropertiesBufferPtr = 0;
        m_HitProxyShaderPtr        = 0;
        m_DeferredContextPtr       = 0;
        m_HitProxyContextPtr       = 0;

        // -----------------------------------------------------------------------------
        // Iterate throw render jobs to release managed pointer
        // -----------------------------------------------------------------------------
        for (auto CurrentRenderJob : m_RefractionRenderJobs)
        {
            CurrentRenderJob.m_SurfacePtr = nullptr;
        }

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLights; ++IndexOfTexture)
        {
            m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture] = nullptr;
        }

        m_ForwardLightTextures.m_DiffuseTexturePtr  = nullptr;
        m_ForwardLightTextures.m_SpecularTexturePtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupShader()
    {
        m_FullscreenVSPtr = ShaderManager::CompileVS("system/vs_fullscreen.glsl", "main");
        m_NormalPSPtr = ShaderManager::CompilePS("refraction/fs_normal.glsl", "main");
        m_NormalTexPSPtr = ShaderManager::CompilePS("refraction/fs_normal.glsl", "main", "#define USE_TEX_NORMAL\n");
        m_RefractionApplyPSPtr = ShaderManager::CompilePS("refraction/fs_apply.glsl", "main");

        m_HitProxyShaderPtr = ShaderManager::CompilePS("picking/fs_hitproxy.glsl", "main");
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupRenderTargets()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupStates()
    {
        CCameraPtr      CameraPtr              = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr ViewPortSetPtr         = ViewManager     ::GetViewPortSet();
        CRenderStatePtr DeferredRenderStatePtr = StateManager    ::GetRenderState(0);
        CRenderStatePtr HitProxyRenderStatePtr = StateManager    ::GetRenderState(CRenderState::EqualDepth);
        CTargetSetPtr   DeferredTargetSetPtr   = TargetSetManager::GetDeferredTargetSet();
        CTargetSetPtr   HitProxyTargetSetPtr   = TargetSetManager::GetHitProxyTargetSet();

        CRenderContextPtr RenderContextPtr = ContextManager::CreateRenderContext();

        RenderContextPtr->SetCamera(CameraPtr);
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(DeferredTargetSetPtr);
        RenderContextPtr->SetRenderState(DeferredRenderStatePtr);

        m_DeferredContextPtr = RenderContextPtr;

        // -----------------------------------------------------------------------------

        RenderContextPtr = ContextManager::CreateRenderContext();

        RenderContextPtr->SetCamera(CameraPtr);
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(HitProxyTargetSetPtr);
        RenderContextPtr->SetRenderState(HitProxyRenderStatePtr);

        m_HitProxyContextPtr = RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupTextures()
    {
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = Main::GetActiveWindowSize()[0];
        TextureDescriptor.m_NumberOfPixelsV  = Main::GetActiveWindowSize()[1];
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R32G32B32_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        
        m_RefractiveNormalTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_RefractiveNormalTexturePtr, "Camera Refractive Normal");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = Main::GetActiveWindowSize()[0];
        TextureDescriptor.m_NumberOfPixelsV  = Main::GetActiveWindowSize()[1];
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::DepthStencilTarget;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R32_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        
        m_RefractiveDepthTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_RefractiveDepthTexturePtr, "Camera Refractive Depth");

        // -----------------------------------------------------------------------------
        // Target Set
        // -----------------------------------------------------------------------------
        m_RefractionTargetSetPtr = TargetSetManager::CreateTargetSet(m_RefractiveNormalTexturePtr, m_RefractiveDepthTexturePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_ModelBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_ModelBufferPtr, "Model Matrix");

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_SurfaceMaterialBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_SurfaceMaterialBufferPtr, "Surface Material Attributes");

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SLightProperties) * s_MaxNumberOfLights;
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_LightPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_LightPropertiesBufferPtr, "Light Properties");

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SForwardPassProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_ForwardPassBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_ForwardPassBufferPtr, "Forward Pass Properties");

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCausticSettingsBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_CausticSettingsBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_CausticSettingsBufferPtr, "Caustic Settings Properties");
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupResources()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupModels()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnSetupEnd()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnNewMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnUnloadMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::Update()
    {
        // -----------------------------------------------------------------------------
        // Set render jobs depending on camera. At the end we have to iterate throw
        // the map only once. Then we can order the render jobs and we get as less
        // state changes as possible.
        // -----------------------------------------------------------------------------
        BuildRenderJobs();

        // -----------------------------------------------------------------------------
        // Update light properties
        // -----------------------------------------------------------------------------
        UpdateLightProperties();
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::RenderForward()
    {
        if (m_RefractionRenderJobs.size() == 0) return;

        Performance::BeginEvent("Refractive Meshes");

        Performance::BeginEvent("Background");

        TargetSetManager::ClearTargetSet(m_RefractionTargetSetPtr, glm::vec4(0.0f), 0.0f);

        ContextManager::SetTargetSet(m_RefractionTargetSetPtr);

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::EState::GreatEqualDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::FrontCull));

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);

        ContextManager::SetConstantBuffer(2, m_SurfaceMaterialBufferPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        for (auto RenderJob : m_RefractionRenderJobs)
        {
            CSurfacePtr SurfacePtr = RenderJob.m_SurfacePtr;

            const Gfx::CMaterial* pMaterial = RenderJob.m_SurfaceMaterialPtr;

            // -----------------------------------------------------------------------------

            ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(1, pMaterial->GetTextureSetPS()->GetTexture(1));

            // -----------------------------------------------------------------------------

            if (pMaterial->GetKey().m_HasNormalTex)
            {
                ContextManager::SetShaderPS(m_NormalTexPSPtr);
            }
            else
            {
                ContextManager::SetShaderPS(m_NormalPSPtr);
            }

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = RenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &pMaterial->GetMaterialAttributes());

            // -----------------------------------------------------------------------------

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }

        Performance::EndEvent();

        Performance::BeginEvent("Apply");

        Debug::Push(131222);

        ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AlphaBlend));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
        ContextManager::SetConstantBuffer(3, m_SurfaceMaterialBufferPtr);
        ContextManager::SetConstantBuffer(4, m_ForwardPassBufferPtr);
        ContextManager::SetConstantBuffer(5, m_CausticSettingsBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());
        ContextManager::SetResourceBuffer(1, m_LightPropertiesBufferPtr);

        // -----------------------------------------------------------------------------
        // Bind shadow and reflection textures
        // -----------------------------------------------------------------------------
        ContextManager::SetSampler(7, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(7, m_RefractiveNormalTexturePtr);

        ContextManager::SetSampler(8, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(8, m_RefractiveDepthTexturePtr);

        ContextManager::SetSampler(9, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(9, ReflectionRenderer::GetBRDF());

        if (m_ForwardLightTextures.m_SpecularTexturePtr != 0)
        {
            ContextManager::SetSampler(10, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(10, m_ForwardLightTextures.m_SpecularTexturePtr);
        }

        if (m_ForwardLightTextures.m_DiffuseTexturePtr != 0)
        {
            ContextManager::SetSampler(11, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(11, m_ForwardLightTextures.m_DiffuseTexturePtr);
        }

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLights; ++IndexOfTexture)
        {
            if (m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture] != 0)
            {
                ContextManager::SetSampler(12 + IndexOfTexture, SamplerManager::GetSampler(CSampler::PCF));

                ContextManager::SetTexture(12 + IndexOfTexture, m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture]);
            }
        }

        // -----------------------------------------------------------------------------
        // Refraction pass properties
        // -----------------------------------------------------------------------------
        SForwardPassProperties ForwardPassProperties;

        ForwardPassProperties.m_CameraPosition = glm::vec4(ViewManager::GetMainCamera()->GetView()->GetPosition(), 1.0f);
        ForwardPassProperties.m_ExposureHistoryIndex = HistogramRenderer::GetCurrentExposureHistoryIndex();

        BufferManager::UploadBufferData(m_ForwardPassBufferPtr, &ForwardPassProperties);

        // -----------------------------------------------------------------------------
        // Actors
        // -----------------------------------------------------------------------------
        for (auto RenderJob : m_RefractionRenderJobs)
        {
            const CMaterial* pMaterial = RenderJob.m_SurfaceMaterialPtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = RenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &pMaterial->GetMaterialAttributes());

            float Near = ViewManager::GetMainCamera()->GetNear();
            float Far = ViewManager::GetMainCamera()->GetFar();
            float IndexOfRefraction = pMaterial->GetMaterialRefractionAttributes().m_IndexOfRefraction;

            SCausticSettingsBuffer CausticSettingsBuffer;

            CausticSettingsBuffer.m_RefractionIndices = glm::vec4(1.0f / IndexOfRefraction, 1.0f / (IndexOfRefraction * IndexOfRefraction), IndexOfRefraction, IndexOfRefraction * IndexOfRefraction);
            CausticSettingsBuffer.m_DepthLinearization = glm::vec4(Near * Far, Far - Near, Far + Near, Far);

            BufferManager::UploadBufferData(m_CausticSettingsBufferPtr, &CausticSettingsBuffer);

            // -----------------------------------------------------------------------------
            // Surface
            // -----------------------------------------------------------------------------
            CSurfacePtr SurfacePtr = RenderJob.m_SurfacePtr;

            // -----------------------------------------------------------------------------
            // Set shader
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderPS(m_RefractionApplyPSPtr);

            // -----------------------------------------------------------------------------
            // Set textures
            // -----------------------------------------------------------------------------
            for (unsigned int IndexOfTexture = 0; IndexOfTexture < pMaterial->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
            {
                ContextManager::SetSampler(IndexOfTexture, pMaterial->GetSamplerSetPS()->GetSampler(IndexOfTexture));

                ContextManager::SetTexture(IndexOfTexture, pMaterial->GetTextureSetPS()->GetTexture(IndexOfTexture));
            }

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }

        Debug::Pop();

        Performance::EndEvent();

        ContextManager::ResetSampler(0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);
        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderDS();

        ContextManager::ResetShaderHS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRasterizerState();

        ContextManager::ResetDepthStencilState();

        ContextManager::ResetViewPortSet();

        ContextManager::ResetTargetSet();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // That is an sort object for sorting render jobs. They will be sorted from
        // lower number to higher number. The surface attribute is symbol for
        // complexity.
        // In the case of forward rendering jobs we sort the jobs depending on the
        // distance between main camera and object.
        // -----------------------------------------------------------------------------
        struct SDistanceSortObject
        {
            bool operator() (SRenderJob& _rLeft, SRenderJob& _rRight)
            {
                glm::vec4 ViewPosition = glm::vec4(ViewManager::GetMainCamera()->GetView()->GetPosition(), 1.0f);

                return (glm::distance(_rLeft.m_ModelMatrix[3], ViewPosition) > glm::distance(_rRight.m_ModelMatrix[3], ViewPosition));
            }
        } DistanceSortObject;

        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_RefractionRenderJobs.clear();

        auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

        for (auto Component : DataMeshComponents)
        {
            Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            const Dt::CEntity& rCurrentEntity = *pDtComponent->GetHostEntity();

            if (rCurrentEntity.GetLayer() == Dt::SEntityLayer::Default)
            {
                Gfx::CMesh* pGfxComponent = static_cast<Gfx::CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------
                // Set every surface of this entity into a new render job
                // -----------------------------------------------------------------------------
                if (pGfxComponent->GetLOD(0) == nullptr) continue;

                CSurfacePtr SurfacePtr = pGfxComponent->GetLOD(0)->GetSurface();

                if (SurfacePtr == nullptr) continue;

                const Gfx::CMaterial* pMaterial = SurfacePtr->GetMaterial();

                if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    pMaterial = static_cast<const Gfx::CMaterial*>(pMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }

                assert(pMaterial != 0);

                if (!pMaterial->HasRefraction()) continue;

                // -----------------------------------------------------------------------------
                // Set information to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_SurfaceAttributes  = SurfacePtr->GetKey().m_Key;
                NewRenderJob.m_EntityID           = rCurrentEntity.GetID();
                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = pMaterial;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                m_RefractionRenderJobs.push_back(NewRenderJob);
            }
        }

        // -----------------------------------------------------------------------------
        // Now we sort the render jobs
        // -----------------------------------------------------------------------------
        std::sort(m_RefractionRenderJobs.begin(), m_RefractionRenderJobs.end(), DistanceSortObject);
    }

    // -----------------------------------------------------------------------------

    void CGfxRefractionRenderer::UpdateLightProperties()
    {
        SLightProperties LightProperties[s_MaxNumberOfLights];
        unsigned int     IndexOfLight;

        // -----------------------------------------------------------------------------
        // Clear textures
        // -----------------------------------------------------------------------------
        m_ForwardLightTextures.m_SpecularTexturePtr = nullptr;
        m_ForwardLightTextures.m_DiffuseTexturePtr  = nullptr;

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLights; ++IndexOfTexture)
        {
            m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture] = nullptr;
        }

        // -----------------------------------------------------------------------------
        // Initiate light properties buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Zero(&LightProperties, sizeof(SLightProperties) * s_MaxNumberOfLights);

        // -----------------------------------------------------------------------------
        // Fill with data
        // -----------------------------------------------------------------------------
        IndexOfLight = 0;

        // -----------------------------------------------------------------------------
        // Suns
        // -----------------------------------------------------------------------------
        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSunComponent>();

        for (auto Component : DataComponents)
        {
            if (IndexOfLight == s_MaxNumberOfLights) break;

            Dt::CSunComponent* pDtComponent = static_cast<Dt::CSunComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            Gfx::CSun* pGfxComponent = static_cast<Gfx::CSun*>(pDtComponent->GetFacet(Dt::CSunComponent::Graphic));

            float HasShadows = 1.0f;

            LightProperties[IndexOfLight].m_LightType           = 1;
            LightProperties[IndexOfLight].m_LightViewProjection = pGfxComponent->GetCamera()->GetViewProjectionMatrix();
            LightProperties[IndexOfLight].m_LightDirection      = glm::normalize(glm::vec4(pDtComponent->GetDirection(), 0.0f));
            LightProperties[IndexOfLight].m_LightColor          = glm::vec4(pDtComponent->GetLightness(), 1.0f);
            LightProperties[IndexOfLight].m_LightSettings       = glm::vec4(pDtComponent->GetSunAngularRadius(), 0.0f, 0.0f, HasShadows);

            // -----------------------------------------------------------------------------

            m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfLight] = pGfxComponent->GetShadowMapPtr();

            // -----------------------------------------------------------------------------

            ++IndexOfLight;
        }

        // -----------------------------------------------------------------------------
        // Point lights
        // -----------------------------------------------------------------------------
        DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CPointLightComponent>();

        for (auto Component : DataComponents)
        {
            if (IndexOfLight == s_MaxNumberOfLights) break;

            Dt::CPointLightComponent* pDtComponent = static_cast<Dt::CPointLightComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            Gfx::CPointLight* pGfxComponent = static_cast<Gfx::CPointLight*>(pDtComponent->GetFacet(Dt::CPointLightComponent::Graphic));

            float InvSqrAttenuationRadius = pDtComponent->GetReciprocalSquaredAttenuationRadius();
            float AngleScale              = pDtComponent->GetAngleScale();
            float AngleOffset             = pDtComponent->GetAngleOffset();
            float HasShadows              = pDtComponent->GetShadowType() != Dt::CPointLightComponent::NoShadows ? 1.0f : 0.0f;

            LightProperties[IndexOfLight].m_LightType      = 2;
            LightProperties[IndexOfLight].m_LightPosition  = glm::vec4(pDtComponent->GetHostEntity()->GetWorldPosition(), 1.0f);
            LightProperties[IndexOfLight].m_LightDirection = glm::normalize(glm::vec4(pDtComponent->GetDirection(), 0.0f));
            LightProperties[IndexOfLight].m_LightColor     = glm::vec4(pDtComponent->GetLightness(), 1.0f);
            LightProperties[IndexOfLight].m_LightSettings  = glm::vec4(InvSqrAttenuationRadius, AngleScale, AngleOffset, HasShadows);

            LightProperties[IndexOfLight].m_LightViewProjection = glm::mat4(1.0f);

            if (pDtComponent->GetShadowType() != Dt::CPointLightComponent::NoShadows)
            {
                assert(pGfxComponent->GetCamera().IsValid());

                LightProperties[IndexOfLight].m_LightViewProjection = pGfxComponent->GetCamera()->GetViewProjectionMatrix();
            }

            // -----------------------------------------------------------------------------

            if (pDtComponent->GetShadowType() != Dt::CPointLightComponent::NoShadows)
            {
                m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfLight] = pGfxComponent->GetTextureSMSet()->GetTexture(0);
            }

            // -----------------------------------------------------------------------------

            ++IndexOfLight;
        }

        // -----------------------------------------------------------------------------
        // Light probe
        // -----------------------------------------------------------------------------
        DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CLightProbeComponent>();

        for (auto Component : DataComponents)
        {
            if (IndexOfLight == s_MaxNumberOfLights) break;

            Dt::CLightProbeComponent* pDtComponent = static_cast<Dt::CLightProbeComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            Gfx::CLightProbe* pGfxComponent = static_cast<Gfx::CLightProbe*>(pDtComponent->GetFacet(Dt::CLightProbeComponent::Graphic));

            LightProperties[IndexOfLight].m_LightType      = 3;
            LightProperties[IndexOfLight].m_LightPosition  = glm::vec4(pDtComponent->GetHostEntity()->GetWorldPosition(), 1.0f);
            LightProperties[IndexOfLight].m_LightDirection = glm::vec4(0.0f);
            LightProperties[IndexOfLight].m_LightColor     = glm::vec4(0.0f);
            LightProperties[IndexOfLight].m_LightSettings  = glm::vec4(static_cast<float>(pGfxComponent->GetSpecularPtr()->GetNumberOfMipLevels() - 1), 0.0f, 0.0f, 0.0f);

            LightProperties[IndexOfLight].m_LightViewProjection = glm::mat4(1.0f);

            m_ForwardLightTextures.m_SpecularTexturePtr = pGfxComponent->GetSpecularPtr();
            m_ForwardLightTextures.m_DiffuseTexturePtr  = pGfxComponent->GetDiffusePtr();

            ++IndexOfLight;
        }

        BufferManager::UploadBufferData(m_LightPropertiesBufferPtr, &LightProperties);


        // TODO by tschwandt (2018/06/14)
        // 1. There is no check included if the light has an effect on the object!
        // 2. Area lights are not supported!
    }
} // namespace


namespace Gfx
{
namespace RefractionRenderer
{
    void OnStart()
    {
        CGfxRefractionRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxRefractionRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void OnSetupShader()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void OnSetupKernels()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupKernels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupRenderTargets()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupRenderTargets();
    }

    // -----------------------------------------------------------------------------

    void OnSetupStates()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupStates();
    }

    // -----------------------------------------------------------------------------

    void OnSetupTextures()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void OnSetupBuffers()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupBuffers();
    }

    // -----------------------------------------------------------------------------

    void OnSetupResources()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupResources();
    }

    // -----------------------------------------------------------------------------

    void OnSetupModels()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupModels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupEnd()
    {
        CGfxRefractionRenderer::GetInstance().OnSetupEnd();
    }

    // -----------------------------------------------------------------------------

    void OnReload()
    {
        CGfxRefractionRenderer::GetInstance().OnReload();
    }

    // -----------------------------------------------------------------------------

    void OnNewMap()
    {
        CGfxRefractionRenderer::GetInstance().OnNewMap();
    }

    // -----------------------------------------------------------------------------

    void OnUnloadMap()
    {
        CGfxRefractionRenderer::GetInstance().OnUnloadMap();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxRefractionRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void RenderForward()
    {
        CGfxRefractionRenderer::GetInstance().RenderForward();
    }
} // namespace RefractionRenderer
} // namespace Gfx
