
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_point_light_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_caustic_renderer.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_point_light.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxCausticRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxCausticRenderer)
        
    public:
        
        CGfxCausticRenderer();
        ~CGfxCausticRenderer();
        
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
        void SetSetting(const SCausticSettings& _rSettings);
        const SCausticSettings& GetSettings();

    private:

        struct SPerLightConstantBuffer
        {
            glm::mat4 vs_ProjectionMatrix;
            glm::mat4 vs_ViewMatrix;
        };

        struct SPerMeshConstantBuffer
        {
            glm::mat4 vs_ModelMatrix;
        };

    private:

        SCausticSettings m_Settings;

        CShaderPtr m_FullscreenVSPtr;
        CShaderPtr m_PhotonEmissionPSPtr;
        CShaderPtr m_PhotonGatheringPSPtr;

        CTexturePtr m_FluxTexturePtr;
        CTexturePtr m_DepthTexturePtr;
        CTexturePtr m_PhotonLocationTexturePtr;

        CTargetSetPtr m_TargetSetPtr;

        CViewPortSetPtr m_ViewportSetPtr;

        CBufferPtr m_PerLightConstantBufferPtr;
        CBufferPtr m_PerMeshConstantBuffer;
    };
} // namespace

namespace
{
    CGfxCausticRenderer::CGfxCausticRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxCausticRenderer::~CGfxCausticRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnExit()
    {
        m_FullscreenVSPtr = 0;
        m_PhotonEmissionPSPtr = 0;
        m_PhotonGatheringPSPtr = 0;
        m_FluxTexturePtr = 0;
        m_DepthTexturePtr = 0;
        m_PhotonLocationTexturePtr = 0;
        m_TargetSetPtr = 0;
        m_ViewportSetPtr = 0;
        m_PerLightConstantBufferPtr = 0;
        m_PerMeshConstantBuffer = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupShader()
    {
        m_FullscreenVSPtr = ShaderManager::CompileVS("system/vs_fullscreen.glsl", "main");
        m_PhotonEmissionPSPtr = ShaderManager::CompilePS("caustic/fs_emission.glsl", "main");
        m_PhotonGatheringPSPtr = ShaderManager::CompilePS("caustic/fs_gathering.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupRenderTargets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupStates()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupTextures()
    {
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
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
        
        m_FluxTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_FluxTexturePtr, "Flux");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
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
        
        m_DepthTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_DepthTexturePtr, "Depth");

        // -----------------------------------------------------------------------------
        
        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
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
        
        m_PhotonLocationTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_PhotonLocationTexturePtr, "Photon Location");

        // -----------------------------------------------------------------------------
        // Target set
        // -----------------------------------------------------------------------------
        m_TargetSetPtr = TargetSetManager::CreateTargetSet(m_FluxTexturePtr, m_PhotonLocationTexturePtr, m_DepthTexturePtr);

        // -----------------------------------------------------------------------------
        // View port
        // -----------------------------------------------------------------------------
        SViewPortDescriptor ViewPortDesc;
        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(512);
        ViewPortDesc.m_Height   = static_cast<float>(512);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        m_ViewportSetPtr = ViewManager::CreateViewPortSet(ViewManager::CreateViewPort(ViewPortDesc));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerLightConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PerLightConstantBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerMeshConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PerMeshConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupResources()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupModels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnReload()
    {
        
    }

    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::Update()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::Render()
    {
        Performance::BeginEvent("Caustics");

        auto PointLightComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CPointLightComponent>();

        for (auto Component : PointLightComponents)
        {
            Dt::CPointLightComponent* pPointLightComponent = static_cast<Dt::CPointLightComponent*>(Component);

            if (pPointLightComponent->IsActiveAndUsable() == false) continue;

            CPointLight* pPointLight = static_cast<CPointLight*>(pPointLightComponent->GetFacet(Dt::CPointLightComponent::Graphic));

            // -----------------------------------------------------------------------------

            SPerLightConstantBuffer ViewBuffer;

            ViewBuffer.vs_ProjectionMatrix = pPointLight->GetCamera()->GetProjectionMatrix();
            ViewBuffer.vs_ViewMatrix       = pPointLight->GetCamera()->GetView()->GetViewMatrix();

            BufferManager::UploadBufferData(m_PerLightConstantBufferPtr, &ViewBuffer);

            // -----------------------------------------------------------------------------

            Performance::BeginEvent("Emission");

            TargetSetManager::ClearTargetSet(m_TargetSetPtr);

            ContextManager::SetTargetSet(m_TargetSetPtr);

            ContextManager::SetViewPortSet(m_ViewportSetPtr);

            ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));

            ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

            ContextManager::SetConstantBuffer(0, m_PerLightConstantBufferPtr);

            ContextManager::SetConstantBuffer(1, m_PerMeshConstantBuffer);

            ContextManager::SetShaderPS(m_PhotonEmissionPSPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

            for (auto Component : DataMeshComponents)
            {
                Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

                if (pDtComponent->IsActiveAndUsable() == false) continue;

                CMesh* pMesh = static_cast<CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------

                CSurfacePtr SurfacePtr = pMesh->GetLOD(0)->GetSurface();

                const Gfx::CMaterial* pMaterial = SurfacePtr->GetMaterial();

                if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pDtMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    pMaterial = static_cast<const Gfx::CMaterial*>(pDtMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }

                // -----------------------------------------------------------------------------

                BufferManager::UploadBufferData(m_PerMeshConstantBuffer, &pDtComponent->GetHostEntity()->GetTransformationFacet()->GetWorldMatrix());

                // -----------------------------------------------------------------------------

                ContextManager::SetShaderVS(SurfacePtr->GetMVPShaderVS());

                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
            }

            ContextManager::ResetTopology();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBuffer();

            ContextManager::ResetConstantBuffer(0);

            ContextManager::ResetConstantBuffer(1);

            ContextManager::ResetConstantBuffer(2);

            ContextManager::ResetConstantBuffer(3);

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetRenderContext();

            Performance::EndEvent();
        }

        Performance::BeginEvent("Gathering");

        ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AdditionBlend));

        ContextManager::SetShaderVS(m_FullscreenVSPtr);

        ContextManager::SetShaderPS(m_PhotonGatheringPSPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, m_PhotonLocationTexturePtr);

        ContextManager::Draw(3, 0);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxCausticRenderer::ResetSettings()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxCausticRenderer::SetSetting(const SCausticSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    const SCausticSettings& CGfxCausticRenderer::GetSettings()
    {
        return m_Settings;
    }
} // namespace


namespace Gfx
{
namespace CausticRenderer
{
    void OnStart()
    {
        CGfxCausticRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxCausticRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    
    void OnSetupShader()
    {
        CGfxCausticRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxCausticRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxCausticRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxCausticRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxCausticRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxCausticRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxCausticRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxCausticRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxCausticRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxCausticRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxCausticRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxCausticRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxCausticRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSetting(const SCausticSettings& _rSettings)
    {
        CGfxCausticRenderer::GetInstance().SetSetting(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SCausticSettings& GetSettings()
    {
        return CGfxCausticRenderer::GetInstance().GetSettings();
    }
} // namespace CausticRenderer
} // namespace Gfx
