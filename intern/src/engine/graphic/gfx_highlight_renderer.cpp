
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_math_limits.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"
#include "engine/data/data_light_probe_component.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_ar_renderer.h"
#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_debug_renderer.h"
#include "engine/graphic/gfx_highlight_renderer.h"
#include "engine/graphic/gfx_light_probe.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_material.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_mesh_renderer.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_reconstruction_renderer.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxHighlightRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxHighlightRenderer)
        
    public:
        
        CGfxHighlightRenderer();
        ~CGfxHighlightRenderer();
        
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
        
        void Update();
        void Render();

        void ResetSettings();
        void SetSettings(const SHighlightSettings& _rSettings);
        const SHighlightSettings& GetSettings() const;

        void HighlightEntity(Base::ID _EntityID);
        void Reset();

    private:

        struct SPerDrawCallConstantBufferVS
        {
            glm::mat4 m_ModelMatrix;
        };
        
        struct SHighlightSettingsBuffer
        {
            glm::vec4 m_ColorAlpha;
        };

        struct SSurfaceRenderJob
        {
            CSurfacePtr m_SurfacePtr;
            glm::mat4   m_ModelMatrix;
        };

        struct SProbeRenderJob
        {
            glm::mat4                 m_ModelMatrix;
            Gfx::CTexturePtr          m_TextureCubePtr;
            Dt::CLightProbeComponent* m_pDtProbeFacet;
        };

    private:

        using CSurfaceRenderJobs = std::vector<SSurfaceRenderJob>;
        using CProbeRenderJobs   = std::vector<SProbeRenderJob>;
        
    private:
        
        CBufferPtr             m_ModelBufferPtr;
        CBufferPtr             m_HighlightPSBufferPtr;
        CShaderPtr             m_HighlightPSPtr;
        CShaderPtr             m_TextureCubePSPtr;
        CShaderPtr             m_SelectionCSPtr;
        CMeshPtr               m_SphereMeshPtr;
        CMeshPtr               m_BoxMeshPtr;
        CSurfaceRenderJobs     m_SurfaceRenderJobs;
        CProbeRenderJobs       m_ProbeRenderJobs;

        Dt::CEntity* m_pSelectedEntity;

        SHighlightSettings m_Settings;
        
    private:

        void RenderHighlightSurfaces();

        void RenderHighlightProbes();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxHighlightRenderer::CGfxHighlightRenderer()
        : m_ModelBufferPtr      ()
        , m_HighlightPSBufferPtr()
        , m_HighlightPSPtr      ()
        , m_TextureCubePSPtr    ()
        , m_SphereMeshPtr       ()
        , m_BoxMeshPtr          ()
        , m_SurfaceRenderJobs   ()
        , m_ProbeRenderJobs     ()
        , m_pSelectedEntity     (nullptr)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxHighlightRenderer::~CGfxHighlightRenderer()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnExit()
    {
        m_ModelBufferPtr       = nullptr;
        m_HighlightPSBufferPtr = nullptr;
        m_HighlightPSPtr       = nullptr;
        m_TextureCubePSPtr     = nullptr;
        m_SelectionCSPtr       = nullptr;
        m_SphereMeshPtr        = nullptr;
        m_BoxMeshPtr           = nullptr;

        // -----------------------------------------------------------------------------
        // Iterate throw render jobs to release managed pointer
        // -----------------------------------------------------------------------------
        CSurfaceRenderJobs::const_iterator EndOfRenderJobs;

        EndOfRenderJobs = m_SurfaceRenderJobs.end();

        for (auto CurrentRenderJob = m_SurfaceRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CurrentRenderJob->m_SurfacePtr = nullptr;
        }

        m_SurfaceRenderJobs.clear();

        m_ProbeRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupShader()
    {
        m_HighlightPSPtr   = ShaderManager::CompilePS("picking/fs_highlight.glsl", "main");
        m_TextureCubePSPtr = ShaderManager::CompilePS("picking/fs_texture_cube.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupRenderTargets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupStates()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;
        
        m_ModelBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHighlightSettingsBuffer);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;

        m_HighlightPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupModels()
    {
        m_SphereMeshPtr = MeshManager::CreateSphere(1.0f, 32, 32);

        m_BoxMeshPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHighlightRenderer::Render()
    {
        if (m_SurfaceRenderJobs.empty()) return;

        Performance::BeginEvent("Highlight");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet(TargetSetManager::GetDefaultTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AlphaBlend));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(m_Settings.m_HighlightUseDepth == true ? CDepthStencilState::LessEqualDepth : 0));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);

        ContextManager::SetConstantBuffer(2, m_HighlightPSBufferPtr);

        // -----------------------------------------------------------------------------
        // Upload settings
        // -----------------------------------------------------------------------------
        SHighlightSettingsBuffer SelectionSettings;

        SelectionSettings.m_ColorAlpha = m_Settings.m_HighlightColor;

        BufferManager::UploadBufferData(m_HighlightPSBufferPtr, &SelectionSettings);

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        RenderHighlightSurfaces();

        RenderHighlightProbes();

        // -----------------------------------------------------------------------------
        // Exit renderer
        // -----------------------------------------------------------------------------
        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetTexture(0);

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

    void CGfxHighlightRenderer::HighlightEntity(Base::ID _EntityID)
    {
        m_pSelectedEntity = Dt::EntityManager::GetEntityByID(_EntityID);

        assert(m_pSelectedEntity != nullptr);
    }

    // -----------------------------------------------------------------------------

    void CGfxHighlightRenderer::Reset()
    {
        m_pSelectedEntity = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxHighlightRenderer::ResetSettings()
    {
        SHighlightSettings Settings;

        Settings.m_HighlightColor = Core::CProgramParameters::GetInstance().Get("graphics:editor:highlight:color", glm::vec4(0.31f, 0.45f, 0.64f, 0.4f));
        Settings.m_HighlightUseDepth = Core::CProgramParameters::GetInstance().Get("graphics:editor:highlight:use_depth", true);
        Settings.m_HighlightUseWireframe = Core::CProgramParameters::GetInstance().Get("graphics:editor:highlight:use_wireframe", true);

        SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGfxHighlightRenderer::SetSettings(const SHighlightSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    const SHighlightSettings& CGfxHighlightRenderer::GetSettings() const
    {
        return m_Settings;
    }

    // -----------------------------------------------------------------------------

    void CGfxHighlightRenderer::RenderHighlightSurfaces()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(m_Settings.m_HighlightUseWireframe == true ? CRasterizerState::Wireframe : 0));

        ContextManager::SetShaderPS(m_HighlightPSPtr);

        for (auto CurrentSurfaceRenderJob : m_SurfaceRenderJobs)
        {
            CSurfacePtr  SurfacePtr = CurrentSurfaceRenderJob.m_SurfacePtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentSurfaceRenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxHighlightRenderer::RenderHighlightProbes()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(0));

        ContextManager::SetShaderPS(m_TextureCubePSPtr);

        for (auto CurrentProbeRenderJob : m_ProbeRenderJobs)
        {
            CSurfacePtr SurfacePtr = m_SphereMeshPtr->GetLOD(0)->GetSurface();

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentProbeRenderJob.m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::SetTexture(0, CurrentProbeRenderJob.m_TextureCubePtr);

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }

        // -----------------------------------------------------------------------------

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

        ContextManager::SetShaderPS(m_HighlightPSPtr);

        for (auto CurrentProbeRenderJob : m_ProbeRenderJobs)
        {
            CSurfacePtr SurfacePtr = m_BoxMeshPtr->GetLOD(0)->GetSurface();

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix  = CurrentProbeRenderJob.m_ModelMatrix;
            ModelBuffer.m_ModelMatrix *= glm::scale(CurrentProbeRenderJob.m_pDtProbeFacet->GetBoxSize());

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxHighlightRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_SurfaceRenderJobs.clear();
        m_ProbeRenderJobs  .clear();

        // -----------------------------------------------------------------------------
        // Is an entity selected
        // -----------------------------------------------------------------------------
        if (m_pSelectedEntity == nullptr) return;

        // -----------------------------------------------------------------------------
        // Add current entity depending on type
        // -----------------------------------------------------------------------------
        auto AddEntityToJobs = [&](Dt::CEntity* _pEntity)
        {
            assert(_pEntity);

            if (_pEntity->GetComponentFacet()->HasComponent<Dt::CMeshComponent>())
            {
                auto* pGfxMesh = static_cast<CMesh*>(_pEntity->GetComponentFacet()->GetComponent<Dt::CMeshComponent>()->GetFacet(Dt::CMeshComponent::Graphic));

                CMaterial* pMaterial = nullptr;

                auto pMaterialComponent = _pEntity->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                if (pMaterialComponent)
                {
                    pMaterial = static_cast<CMaterial*>(pMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }
 
                assert(pGfxMesh != nullptr);

                CMeshPtr MeshPtr = pGfxMesh;

                assert(MeshPtr.IsValid());

                // -----------------------------------------------------------------------------
                // Set every surface of this entity into a new render job
                // -----------------------------------------------------------------------------
                CSurfacePtr SurfacePtr = MeshPtr->GetLOD(0)->GetSurface();

                CMaterialPtr MaterialPtr = SurfacePtr->GetMaterial();

                if (pMaterial != nullptr)
                {
                    MaterialPtr = pMaterial;
                }

                assert(MaterialPtr != nullptr && MaterialPtr.IsValid());

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SSurfaceRenderJob NewRenderJob;

                NewRenderJob.m_SurfacePtr  = SurfacePtr;
                NewRenderJob.m_ModelMatrix = _pEntity->GetTransformationFacet()->GetWorldMatrix();

                m_SurfaceRenderJobs.push_back(NewRenderJob);
            }
            else if (_pEntity->GetComponentFacet()->HasComponent<Dt::CLightProbeComponent>())
            {
                auto* pDataComponent = _pEntity->GetComponentFacet()->GetComponent<Dt::CLightProbeComponent>();

                auto* pGfxComponent  = static_cast<Gfx::CLightProbe*>(pDataComponent->GetFacet(Dt::CLightProbeComponent::Graphic));

                assert(pGfxComponent != nullptr);

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SProbeRenderJob NewRenderJob;

                NewRenderJob.m_ModelMatrix    = _pEntity->GetTransformationFacet()->GetWorldMatrix();
                NewRenderJob.m_TextureCubePtr = pGfxComponent->GetSpecularPtr();
                NewRenderJob.m_pDtProbeFacet  = pDataComponent;

                m_ProbeRenderJobs.push_back(NewRenderJob);
            }
        };
        
        std::function<void(Dt::CEntity*)> NavigateToNextEntity = [&](Dt::CEntity* _pRootEntity) -> void
        {
            Dt::CEntity* pSiblingEntity = _pRootEntity;

            for (; pSiblingEntity != nullptr; )
            {
                assert(pSiblingEntity->GetHierarchyFacet());

                Dt::CEntity* pFirstChild = pSiblingEntity->GetHierarchyFacet()->GetFirstChild();

                if (pFirstChild != nullptr)
                {
                    NavigateToNextEntity(pFirstChild);
                }

                if (pSiblingEntity->IsActive())
                {
                    AddEntityToJobs(pSiblingEntity);
                }

                pSiblingEntity = pSiblingEntity->GetHierarchyFacet()->GetSibling();
            }
        };

        if (m_pSelectedEntity != nullptr)
        {
            Dt::CHierarchyFacet* pHierarchyFacet = m_pSelectedEntity->GetHierarchyFacet();

            if (pHierarchyFacet)
            {
                Dt::CEntity* pFirstChild = pHierarchyFacet->GetFirstChild();

                if (pFirstChild != nullptr)
                {
                    NavigateToNextEntity(pFirstChild);
                }
            }

            AddEntityToJobs(m_pSelectedEntity);
        }
    }
} // namespace


namespace Gfx
{
namespace HighlightRenderer
{
    void OnStart()
    {
        CGfxHighlightRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxHighlightRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    
    void OnSetupShader()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxHighlightRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxHighlightRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxHighlightRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxHighlightRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void HighlightEntity(Base::ID _EntityID)
    {
        CGfxHighlightRenderer::GetInstance().HighlightEntity(_EntityID);
    }

    // -----------------------------------------------------------------------------

    void Reset()
    {
        CGfxHighlightRenderer::GetInstance().Reset();
    }
    
    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxHighlightRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SHighlightSettings& _rSettings)
    {
        CGfxHighlightRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SHighlightSettings& GetSettings()
    {
        return CGfxHighlightRenderer::GetInstance().GetSettings();
    }
} // namespace SelectionRenderer
} // namespace Gfx
