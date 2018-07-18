
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
    class CGfxMeshRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxMeshRenderer)

    public:

        CGfxMeshRenderer();
        ~CGfxMeshRenderer();

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
        void RenderForward();
        void RenderHitProxy();

    private:

        static const unsigned int s_MaxNumberOfLights = 4;

    private:

        struct SPerDrawCallConstantBufferVS
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SHitProxyProperties
        {
            unsigned int m_ID;
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

        CBufferPtr        m_ModelBufferPtr;
        CBufferPtr        m_SurfaceMaterialBufferPtr;
        CBufferPtr        m_HitProxyPassPSBufferPtr;
        CBufferPtr        m_ForwardPassBufferPtr;
        CBufferPtr        m_LightPropertiesBufferPtr;
        CShaderPtr        m_HitProxyShaderPtr;
        CRenderContextPtr m_DeferredContextPtr;
        CRenderContextPtr m_HitProxyContextPtr;
        CRenderJobs       m_DeferredRenderJobs;
        CRenderJobs       m_ForwardRenderJobs;
        CRenderJobs       m_RefractionRenderJobs;
        SLightJob         m_ForwardLightTextures;

    private:

        void BuildRenderJobs();
        void UpdateLightProperties();
    };
} // namespace

namespace
{
    CGfxMeshRenderer::CGfxMeshRenderer()
        : m_ModelBufferPtr          ()
        , m_SurfaceMaterialBufferPtr()
        , m_HitProxyPassPSBufferPtr ()
        , m_ForwardPassBufferPtr    ()
        , m_LightPropertiesBufferPtr()
        , m_HitProxyShaderPtr       ()
        , m_DeferredContextPtr      ()
        , m_HitProxyContextPtr      ()
        , m_DeferredRenderJobs      ()
        , m_ForwardRenderJobs       ()
        , m_ForwardLightTextures    ()
    {
        // -----------------------------------------------------------------------------
        // Reserve some jobs
        // -----------------------------------------------------------------------------
        m_DeferredRenderJobs.reserve(256);
        m_ForwardRenderJobs.reserve(32);
        m_RefractionRenderJobs.reserve(4);
    }

    // -----------------------------------------------------------------------------

    CGfxMeshRenderer::~CGfxMeshRenderer()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnExit()
    {
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
        for (auto CurrentRenderJob : m_DeferredRenderJobs)
        {
            CurrentRenderJob.m_SurfacePtr = nullptr;
        }

        m_DeferredRenderJobs.clear();

        // -----------------------------------------------------------------------------

        for (auto CurrentRenderJob : m_ForwardRenderJobs)
        {
            CurrentRenderJob.m_SurfacePtr = nullptr;
        }

        m_ForwardRenderJobs.clear();

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLights; ++IndexOfTexture)
        {
            m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture] = nullptr;
        }

        m_ForwardLightTextures.m_DiffuseTexturePtr  = nullptr;
        m_ForwardLightTextures.m_SpecularTexturePtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupShader()
    {
        m_HitProxyShaderPtr = ShaderManager::CompilePS("picking/fs_hitproxy.glsl", "main");
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupRenderTargets()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupStates()
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

    void CGfxMeshRenderer::OnSetupTextures()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupBuffers()
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
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHitProxyProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_HitProxyPassPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_HitProxyPassPSBufferPtr, "Hit Proxy Properties");

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
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupResources()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupModels()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnSetupEnd()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnNewMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::OnUnloadMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::Update()
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

    void CGfxMeshRenderer::Render()
    {
        if (m_DeferredRenderJobs.size() == 0) return;

        Performance::BeginEvent("Meshes");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_DeferredContextPtr);

        // -----------------------------------------------------------------------------
        // Iterate throw render jobs and render all meshes
        // -----------------------------------------------------------------------------
        for (auto CurrentRenderJob : m_DeferredRenderJobs)
        {
            CSurfacePtr SurfacePtr = CurrentRenderJob.m_SurfacePtr;

            const CMaterial* pMaterial = CurrentRenderJob.m_SurfaceMaterialPtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentRenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &pMaterial->GetMaterialAttributes());

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderGS(pMaterial->GetShaderGS());

            ContextManager::SetShaderPS(pMaterial->GetShaderPS());

            for (unsigned int IndexOfTexture = 0; IndexOfTexture < pMaterial->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
            {
                ContextManager::SetSampler(IndexOfTexture, pMaterial->GetSamplerSetPS()->GetSampler(IndexOfTexture));

                ContextManager::SetTexture(IndexOfTexture, pMaterial->GetTextureSetPS()->GetTexture(IndexOfTexture));
            }

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
            ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
            ContextManager::SetConstantBuffer(2, m_SurfaceMaterialBufferPtr);

            // -----------------------------------------------------------------------------
            // If we have a bump map we use tessellation
            // TODO: we have to set buffer to shader on model loading
            // TODO: remove GetHasBump()
            // -----------------------------------------------------------------------------
            if (pMaterial->HasBump())
            {
                ContextManager::SetShaderDS(pMaterial->GetShaderDS());

                ContextManager::SetShaderHS(pMaterial->GetShaderHS());

                ContextManager::SetTopology(STopology::Patches);
            }

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

            for (unsigned int IndexOfTexture = 0; IndexOfTexture < pMaterial->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
            {
                ContextManager::ResetSampler(IndexOfTexture);

                ContextManager::ResetTexture(IndexOfTexture);
            }
        }

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderDS();

        ContextManager::ResetShaderHS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        ContextManager::ResetTopology();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::RenderForward()
    {
        if (m_ForwardRenderJobs.size() == 0) return;

        Performance::BeginEvent("Meshes");

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

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());
        ContextManager::SetResourceBuffer(1, m_LightPropertiesBufferPtr);

        // -----------------------------------------------------------------------------
        // Bind shadow and reflection textures
        // -----------------------------------------------------------------------------
        ContextManager::SetSampler(7, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(7, ReflectionRenderer::GetBRDF());

        if (m_ForwardLightTextures.m_SpecularTexturePtr != 0)
        {
            ContextManager::SetSampler(8, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(8, m_ForwardLightTextures.m_SpecularTexturePtr);
        }

        if (m_ForwardLightTextures.m_DiffuseTexturePtr != 0)
        {
            ContextManager::SetSampler(9, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(9, m_ForwardLightTextures.m_DiffuseTexturePtr);
        }

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLights; ++IndexOfTexture)
        {
            if (m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture] != 0)
            {
                ContextManager::SetSampler(10 + IndexOfTexture, SamplerManager::GetSampler(CSampler::PCF));

                ContextManager::SetTexture(10 + IndexOfTexture, m_ForwardLightTextures.m_ShadowTexturePtrs[IndexOfTexture]);
            }
        }

        // -----------------------------------------------------------------------------
        // Forward pass properties
        // -----------------------------------------------------------------------------
        SForwardPassProperties ForwardPassProperties;

        ForwardPassProperties.m_CameraPosition       = glm::vec4(ViewManager::GetMainCamera()->GetView()->GetPosition(), 1.0f);
        ForwardPassProperties.m_ExposureHistoryIndex = HistogramRenderer::GetCurrentExposureHistoryIndex();

        BufferManager::UploadBufferData(m_ForwardPassBufferPtr, &ForwardPassProperties);

        // -----------------------------------------------------------------------------
        // Actors
        // -----------------------------------------------------------------------------
        for (auto RenderJob : m_ForwardRenderJobs)
        {
            const CMaterial* pMaterial = RenderJob.m_SurfaceMaterialPtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = RenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &pMaterial->GetMaterialAttributes());

            // -----------------------------------------------------------------------------
            // Surface
            // -----------------------------------------------------------------------------
            CSurfacePtr SurfacePtr = RenderJob.m_SurfacePtr;

            // -----------------------------------------------------------------------------
            // Set shader
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderPS(pMaterial->GetForwardShaderPS());

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

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < 16; ++IndexOfTexture)
        {
            ContextManager::ResetSampler(IndexOfTexture);

            ContextManager::ResetTexture(IndexOfTexture);
        }

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

        Debug::Pop();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::RenderHitProxy()
    {
        if (m_DeferredRenderJobs.size() == 0) return;

        Performance::BeginEvent("Mesh Hit Proxy");

        ContextManager::SetRenderContext(m_HitProxyContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        auto RenderJobs = [&](const auto _ListOfJobs)->void
        {
            for (auto CurrentRenderJob : _ListOfJobs)
            {
                CSurfacePtr  SurfacePtr = CurrentRenderJob.m_SurfacePtr;

                // -----------------------------------------------------------------------------
                // Upload data to buffer
                // -----------------------------------------------------------------------------
                SPerDrawCallConstantBufferVS ModelBuffer;

                ModelBuffer.m_ModelMatrix = CurrentRenderJob.m_ModelMatrix;

                BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

                SHitProxyProperties HitProxyProperties;

                HitProxyProperties.m_ID = static_cast<unsigned int>(CurrentRenderJob.m_EntityID);

                BufferManager::UploadBufferData(m_HitProxyPassPSBufferPtr, &HitProxyProperties);

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

                ContextManager::SetShaderPS(m_HitProxyShaderPtr);

                ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
                ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
                ContextManager::SetConstantBuffer(2, m_HitProxyPassPSBufferPtr);

                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
            }
        };
        
        RenderJobs(m_DeferredRenderJobs);

        RenderJobs(m_ForwardRenderJobs);

        RenderJobs(m_RefractionRenderJobs);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetShaderPS();

        ContextManager::ResetShaderVS();

        ContextManager::ResetTopology();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // That is an sort object for sorting render jobs. They will be sorted from
        // lower number to higher number. The surface attribute is symbol for
        // complexity.
        // In the case of forward rendering jobs we sort the jobs depending on the
        // distance between main camera and object.
        // -----------------------------------------------------------------------------
        struct SSurfaceSortObject
        {
            bool operator() (SRenderJob& _rLeft, SRenderJob& _rRight)
            {
                return (_rLeft.m_SurfaceAttributes < _rRight.m_SurfaceAttributes);
            }
        } SurfaceSortObject;

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
        m_DeferredRenderJobs.clear();

        m_ForwardRenderJobs.clear();

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

                // -----------------------------------------------------------------------------
                // Set information to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_SurfaceAttributes  = SurfacePtr->GetKey().m_Key;
                NewRenderJob.m_EntityID           = rCurrentEntity.GetID();
                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = pMaterial;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                if (pMaterial->HasRefraction())
                {
                    m_RefractionRenderJobs.push_back(NewRenderJob);
                }
                else if(pMaterial->HasAlpha())
                {
                    m_ForwardRenderJobs.push_back(NewRenderJob);
                }
                else
                {
                    m_DeferredRenderJobs.push_back(NewRenderJob);
                }
            }
        }

        // -----------------------------------------------------------------------------
        // Now we sort the render jobs
        // -----------------------------------------------------------------------------
        std::sort(m_DeferredRenderJobs.begin(), m_DeferredRenderJobs.end(), SurfaceSortObject);

        std::sort(m_ForwardRenderJobs.begin(), m_ForwardRenderJobs.end(), DistanceSortObject);
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshRenderer::UpdateLightProperties()
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
namespace MeshRenderer
{
    void OnStart()
    {
        CGfxMeshRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxMeshRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void OnSetupShader()
    {
        CGfxMeshRenderer::GetInstance().OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void OnSetupKernels()
    {
        CGfxMeshRenderer::GetInstance().OnSetupKernels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupRenderTargets()
    {
        CGfxMeshRenderer::GetInstance().OnSetupRenderTargets();
    }

    // -----------------------------------------------------------------------------

    void OnSetupStates()
    {
        CGfxMeshRenderer::GetInstance().OnSetupStates();
    }

    // -----------------------------------------------------------------------------

    void OnSetupTextures()
    {
        CGfxMeshRenderer::GetInstance().OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void OnSetupBuffers()
    {
        CGfxMeshRenderer::GetInstance().OnSetupBuffers();
    }

    // -----------------------------------------------------------------------------

    void OnSetupResources()
    {
        CGfxMeshRenderer::GetInstance().OnSetupResources();
    }

    // -----------------------------------------------------------------------------

    void OnSetupModels()
    {
        CGfxMeshRenderer::GetInstance().OnSetupModels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupEnd()
    {
        CGfxMeshRenderer::GetInstance().OnSetupEnd();
    }

    // -----------------------------------------------------------------------------

    void OnReload()
    {
        CGfxMeshRenderer::GetInstance().OnReload();
    }

    // -----------------------------------------------------------------------------

    void OnNewMap()
    {
        CGfxMeshRenderer::GetInstance().OnNewMap();
    }

    // -----------------------------------------------------------------------------

    void OnUnloadMap()
    {
        CGfxMeshRenderer::GetInstance().OnUnloadMap();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxMeshRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CGfxMeshRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void RenderForward()
    {
        CGfxMeshRenderer::GetInstance().RenderForward();
    }

    // -----------------------------------------------------------------------------

    void RenderHitProxy()
    {
        CGfxMeshRenderer::GetInstance().RenderHitProxy();
    }
} // namespace MeshRenderer
} // namespace Gfx
