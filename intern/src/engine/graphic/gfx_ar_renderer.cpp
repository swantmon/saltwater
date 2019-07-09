
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_game_control.h"

#include "engine/core/core_console.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_map.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_ar_renderer.h"
#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_debug.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_material.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_sun.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxARRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxARRenderer)

    public:

        CGfxARRenderer();
        ~CGfxARRenderer();

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
        void RenderHitProxy();

    private:

        struct SPerDrawCallConstantBufferVS
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SBilateralBlurConstantBufferCS
        {
            glm::uvec4 m_Direction;
        };

        struct SHitProxyProperties
        {
            unsigned int m_ID;
        };

        struct SRenderJob
        {
            Base::ID         m_EntityID;
            CSurfacePtr      m_SurfacePtr;
            const CMaterial* m_SurfaceMaterialPtr;
            glm::mat4        m_ModelMatrix;
        };

    private:

        using CRenderJobs = std::vector<SRenderJob>;

    private:

        CBufferPtr m_ModelBufferPtr;
        CBufferPtr m_MaterialPSBufferPtr;
        CBufferPtr m_HitProxyPassPSBufferPtr;
        CRenderContextPtr m_DeferredRenderContextPtr;

        CShaderPtr m_DifferentialGBufferShaderPSPtr;
        CShaderPtr m_HitProxyShaderPtr;

        CRenderJobs m_RenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxARRenderer::CGfxARRenderer()
        : m_ModelBufferPtr                ()
        , m_MaterialPSBufferPtr           ()
        , m_HitProxyPassPSBufferPtr       ()
        , m_DeferredRenderContextPtr      ()
        , m_DifferentialGBufferShaderPSPtr()
        , m_HitProxyShaderPtr             ()
        , m_RenderJobs                    ()
    {
        // -----------------------------------------------------------------------------
        // Reserve some jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.reserve(2);
    }

    // -----------------------------------------------------------------------------

    CGfxARRenderer::~CGfxARRenderer()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnExit()
    {
        m_ModelBufferPtr                 = nullptr;
        m_MaterialPSBufferPtr            = nullptr;
        m_HitProxyPassPSBufferPtr        = nullptr;
        m_DeferredRenderContextPtr       = nullptr;
        m_DifferentialGBufferShaderPSPtr = nullptr;
        m_HitProxyShaderPtr              = nullptr;

        // -----------------------------------------------------------------------------
        // Iterate throw render jobs to release managed pointer
        // -----------------------------------------------------------------------------
        for (auto& rCurrentRenderJob : m_RenderJobs)
        {
            rCurrentRenderJob.m_SurfacePtr = nullptr;
        }

        m_RenderJobs.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupShader()
    {
        m_DifferentialGBufferShaderPSPtr = ShaderManager::CompilePS("ar/fs_differential_gbuffer.glsl", "main");

        m_HitProxyShaderPtr = ShaderManager::CompilePS("picking/fs_hitproxy.glsl", "main");
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupRenderTargets()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupStates()
    {
        CCameraPtr      CameraPtr              = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr ViewPortSetPtr         = ViewManager     ::GetViewPortSet();
        CRenderStatePtr DeferredRenderStatePtr = StateManager    ::GetRenderState(0);
        CTargetSetPtr   DeferredTargetSetPtr   = TargetSetManager::GetDeferredTargetSet();

        // -----------------------------------------------------------------------------

        m_DeferredRenderContextPtr = ContextManager::CreateRenderContext();

        m_DeferredRenderContextPtr->SetCamera(CameraPtr);
        m_DeferredRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_DeferredRenderContextPtr->SetTargetSet(DeferredTargetSetPtr);
        m_DeferredRenderContextPtr->SetRenderState(DeferredRenderStatePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupTextures()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;

        m_ModelBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;

        m_MaterialPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHitProxyProperties);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;

        m_HitProxyPassPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupResources()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupModels()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupEnd()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnNewMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnUnloadMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::Update()
    {
        BuildRenderJobs();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::Render()
    {
        if (m_RenderJobs.empty()) return;

        auto DataCameraComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CCameraComponent>();

        Gfx::CTexturePtr BackgroundTexturePtr = nullptr;

        for (auto Component : DataCameraComponents)
        {
            auto* pDtComponent = static_cast<Dt::CCameraComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            BackgroundTexturePtr = pDtComponent->GetBackgroundTexture();
        }

        if (BackgroundTexturePtr == nullptr) return;

        Performance::BeginEvent("AR: Differential GBuffer");

        ContextManager::SetRenderContext(m_DeferredRenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderPS(m_DifferentialGBufferShaderPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
        ContextManager::SetConstantBuffer(2, m_MaterialPSBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, BackgroundTexturePtr);

        CRenderJobs::const_iterator EndOfRenderJobs = m_RenderJobs.end();

        for (auto& rCurrentRenderJob : m_RenderJobs)
        {
            CSurfacePtr SurfacePtr = rCurrentRenderJob.m_SurfacePtr;

            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = rCurrentRenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            BufferManager::UploadBufferData(m_MaterialPSBufferPtr, &rCurrentRenderJob.m_SurfaceMaterialPtr->GetMaterialAttributes());

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetSampler(0);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        ContextManager::ResetTopology();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::RenderHitProxy()
    {
        if (m_RenderJobs.empty()) return;

        Performance::BeginEvent("AR Hit Proxy");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderPS(m_HitProxyShaderPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
        ContextManager::SetConstantBuffer(2, m_HitProxyPassPSBufferPtr);

        // -----------------------------------------------------------------------------
        // First pass: iterate throw render jobs and compute all meshes
        // -----------------------------------------------------------------------------
        for (auto CurrentRenderJob : m_RenderJobs)
        {
            CSurfacePtr  SurfacePtr = CurrentRenderJob.m_SurfacePtr;

            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentRenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            SHitProxyProperties HitProxyProperties;

            HitProxyProperties.m_ID = static_cast<unsigned int>(CurrentRenderJob.m_EntityID);

            BufferManager::UploadBufferData(m_HitProxyPassPSBufferPtr, &HitProxyProperties);

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }

        ContextManager::ResetShaderPS();

        ContextManager::ResetShaderVS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::BuildRenderJobs()
    {
        m_RenderJobs.clear();

        auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

        for (auto Component : DataMeshComponents)
        {
            auto* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            const Dt::CEntity& rCurrentEntity = *pDtComponent->GetHostEntity();

            if (rCurrentEntity.GetLayer() & Dt::SEntityLayer::AR)
            {
                auto* pGfxComponent = static_cast<Gfx::CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------
                // Surface
                // -----------------------------------------------------------------------------
                CSurfacePtr SurfacePtr = pGfxComponent->GetLOD(0)->GetSurface();

                if (SurfacePtr == nullptr) continue;

                const CMaterial* pMaterial = SurfacePtr->GetMaterial();
                
                if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pMaterialComponent = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    if (pMaterialComponent->IsActiveAndUsable())
                    {
                        pMaterial = static_cast<const Gfx::CMaterial*>(pMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                    }
                }

                assert(pMaterial != nullptr);

                // -----------------------------------------------------------------------------
                // Set information to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_EntityID           = rCurrentEntity.GetID();
                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = pMaterial;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                m_RenderJobs.push_back(NewRenderJob);
            }
        }
    }
} // namespace

namespace Gfx
{
namespace ARRenderer
{
    void OnStart()
    {
        CGfxARRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxARRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void OnSetupShader()
    {
        CGfxARRenderer::GetInstance().OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void OnSetupKernels()
    {
        CGfxARRenderer::GetInstance().OnSetupKernels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupRenderTargets()
    {
        CGfxARRenderer::GetInstance().OnSetupRenderTargets();
    }

    // -----------------------------------------------------------------------------

    void OnSetupStates()
    {
        CGfxARRenderer::GetInstance().OnSetupStates();
    }

    // -----------------------------------------------------------------------------

    void OnSetupTextures()
    {
        CGfxARRenderer::GetInstance().OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void OnSetupBuffers()
    {
        CGfxARRenderer::GetInstance().OnSetupBuffers();
    }

    // -----------------------------------------------------------------------------

    void OnSetupResources()
    {
        CGfxARRenderer::GetInstance().OnSetupResources();
    }

    // -----------------------------------------------------------------------------

    void OnSetupModels()
    {
        CGfxARRenderer::GetInstance().OnSetupModels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupEnd()
    {
        CGfxARRenderer::GetInstance().OnSetupEnd();
    }

    // -----------------------------------------------------------------------------

    void OnReload()
    {
        CGfxARRenderer::GetInstance().OnReload();
    }

    // -----------------------------------------------------------------------------

    void OnNewMap()
    {
        CGfxARRenderer::GetInstance().OnNewMap();
    }

    // -----------------------------------------------------------------------------

    void OnUnloadMap()
    {
        CGfxARRenderer::GetInstance().OnUnloadMap();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxARRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CGfxARRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void RenderHitProxy()
    {
        CGfxARRenderer::GetInstance().RenderHitProxy();
    }
} // namespace ARRenderer
} // namespace Gfx
