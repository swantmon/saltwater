
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
#include "engine/graphic/gfx_histogram_renderer.h"
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

        static const int s_CausticMapSize = 2048;

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

        struct SLightPropertiesBuffer
        {
            glm::mat4 m_LightProjectionMatrix;
            glm::mat4 m_LightViewMatrix;
            glm::vec4 m_LightColor;
            int       m_ExposureHistoryIndex;
        };

        struct SCausticSettingsBuffer
        {
            glm::vec4 m_RefractionIndices;
            glm::vec4 m_DepthLinearization;
        };

    private:

        SCausticSettings m_Settings;

        CShaderPtr m_FullscreenVSPtr;
        CShaderPtr m_NormalPSPtr;
        CShaderPtr m_NormalTexPSPtr;
        CShaderPtr m_PhotonEmissionPSPtr;
        CShaderPtr m_PhotonGatheringVSPtr;
        CShaderPtr m_PhotonGatheringPSPtr;
        CShaderPtr m_PhotonApplyPSPtr;

        CTexturePtr m_RefractiveNormalTexturePtr;
        CTexturePtr m_RefractiveDepthTexturePtr;
        CTexturePtr m_BackgroundDepthTexturePtr;
        CTexturePtr m_PhotonEmissionDepthTexturePtr;

        CTexturePtr m_PhotonLocationTexturePtr;
        CTexturePtr m_PhotonGatheringTexturePtr;

        CTargetSetPtr m_RefractionTargetSetPtr;
        CTargetSetPtr m_BackgroundTargetSetPtr;
        CTargetSetPtr m_PhotonEmissionTargetSetPtr;
        CTargetSetPtr m_PhotonGatheringTargetSetPtr;

        CViewPortSetPtr m_ViewportSetPtr;

        CBufferPtr m_PerLightConstantBufferPtr;
        CBufferPtr m_PerMeshConstantBufferPtr;
        CBufferPtr m_LightPropertiesBufferPtr;
        CBufferPtr m_SurfaceMaterialBufferPtr;
        CBufferPtr m_CausticSettingsBufferPtr;
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
        m_NormalPSPtr = 0;
        m_NormalTexPSPtr = 0;
        m_PhotonEmissionPSPtr = 0;
        m_PhotonGatheringVSPtr = 0;
        m_PhotonGatheringPSPtr = 0;
        m_PhotonApplyPSPtr = 0;
        m_RefractiveNormalTexturePtr = 0;
        m_RefractiveDepthTexturePtr = 0;
        m_BackgroundDepthTexturePtr = 0;
        m_PhotonLocationTexturePtr = 0;
        m_PhotonGatheringTexturePtr = 0;
        m_PhotonEmissionDepthTexturePtr = 0;
        m_RefractionTargetSetPtr = 0;
        m_BackgroundTargetSetPtr = 0;
        m_PhotonEmissionTargetSetPtr = 0;
        m_PhotonGatheringTargetSetPtr = 0;
        m_ViewportSetPtr = 0;
        m_PerLightConstantBufferPtr = 0;
        m_PerMeshConstantBufferPtr = 0;
        m_LightPropertiesBufferPtr = 0;
        m_SurfaceMaterialBufferPtr = 0;
        m_CausticSettingsBufferPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupShader()
    {
        std::string Define = "";
        Define += "#define CAUSTIC_MAP_RESOLUTION " + std::to_string(s_CausticMapSize) + "\n";
        Define += "#define PHOTON_RESOLUTION_MULTIPLIER 0.001f\n";

        m_FullscreenVSPtr = ShaderManager::CompileVS("system/vs_fullscreen.glsl", "main");
        m_NormalPSPtr = ShaderManager::CompilePS("caustic/fs_normal.glsl", "main");
        m_NormalTexPSPtr = ShaderManager::CompilePS("caustic/fs_normal.glsl", "main", "#define USE_TEX_NORMAL\n");
        m_PhotonEmissionPSPtr = ShaderManager::CompilePS("caustic/fs_emission.glsl", "main", Define.c_str());
        m_PhotonGatheringVSPtr = ShaderManager::CompileVS("caustic/vs_gathering.glsl", "main", Define.c_str());
        m_PhotonGatheringPSPtr = ShaderManager::CompilePS("caustic/fs_gathering.glsl", "main", Define.c_str());
        m_PhotonApplyPSPtr = ShaderManager::CompilePS("caustic/fs_apply.glsl", "main");
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

        TextureDescriptor.m_NumberOfPixelsU  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsV  = s_CausticMapSize;
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

        TextureManager::SetTextureLabel(m_RefractiveNormalTexturePtr, "Refractive Normal");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsV  = s_CausticMapSize;
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

        TextureManager::SetTextureLabel(m_RefractiveDepthTexturePtr, "Refractive Depth");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsV  = s_CausticMapSize;
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
        
        m_PhotonEmissionDepthTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_RefractiveDepthTexturePtr, "Photon Emission Depth");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsV  = s_CausticMapSize;
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
        
        m_BackgroundDepthTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_BackgroundDepthTexturePtr, "Background Depth");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsV  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R32G32B32A32_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        
        m_PhotonLocationTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_PhotonLocationTexturePtr, "Photon Location");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsV  = s_CausticMapSize;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R32G32B32A32_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        
        m_PhotonGatheringTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_PhotonGatheringTexturePtr, "Photon Gathering");

        // -----------------------------------------------------------------------------
        // Target set
        // -----------------------------------------------------------------------------
        m_RefractionTargetSetPtr = TargetSetManager::CreateTargetSet(m_RefractiveNormalTexturePtr, m_RefractiveDepthTexturePtr);

        m_BackgroundTargetSetPtr = TargetSetManager::CreateTargetSet(m_BackgroundDepthTexturePtr);

        m_PhotonEmissionTargetSetPtr = TargetSetManager::CreateTargetSet(m_PhotonLocationTexturePtr, m_PhotonEmissionDepthTexturePtr);

        m_PhotonGatheringTargetSetPtr = TargetSetManager::CreateTargetSet(m_PhotonGatheringTexturePtr);

        // -----------------------------------------------------------------------------
        // View port
        // -----------------------------------------------------------------------------
        SViewPortDescriptor ViewPortDesc;
        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(s_CausticMapSize);
        ViewPortDesc.m_Height   = static_cast<float>(s_CausticMapSize);
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

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerMeshConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PerMeshConstantBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SLightPropertiesBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_LightPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_SurfaceMaterialBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCausticSettingsBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_CausticSettingsBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
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

             SLightPropertiesBuffer LightProperties; 
 
            float InvSqrAttenuationRadius = pPointLightComponent->GetReciprocalSquaredAttenuationRadius(); 
            float AngleScale              = pPointLightComponent->GetAngleScale(); 
            float AngleOffset             = pPointLightComponent->GetAngleOffset(); 
 
            LightProperties.m_LightProjectionMatrix = pPointLight->GetCamera()->GetProjectionMatrix();
            LightProperties.m_LightViewMatrix       = pPointLight->GetCamera()->GetView()->GetViewMatrix();
            LightProperties.m_LightColor            = glm::vec4(pPointLightComponent->GetLightness(), 1.0f);
            LightProperties.m_ExposureHistoryIndex  = HistogramRenderer::GetCurrentExposureHistoryIndex();
 
            BufferManager::UploadBufferData(m_LightPropertiesBufferPtr, &LightProperties); 

            // -----------------------------------------------------------------------------

            SPerLightConstantBuffer ViewBuffer;

            ViewBuffer.vs_ProjectionMatrix = pPointLight->GetCamera()->GetProjectionMatrix();
            ViewBuffer.vs_ViewMatrix       = pPointLight->GetCamera()->GetView()->GetViewMatrix();

            BufferManager::UploadBufferData(m_PerLightConstantBufferPtr, &ViewBuffer);

            // -----------------------------------------------------------------------------

            Performance::BeginEvent("Refraction");

            TargetSetManager::ClearTargetSet(m_RefractionTargetSetPtr, glm::vec4(0.0f), 0.0f);

            ContextManager::SetTargetSet(m_RefractionTargetSetPtr);

            ContextManager::SetViewPortSet(m_ViewportSetPtr);

            ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::EState::GreatEqualDepth));

            ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::FrontCull));

            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

            ContextManager::SetConstantBuffer(0, m_PerLightConstantBufferPtr);

            ContextManager::SetConstantBuffer(1, m_PerMeshConstantBufferPtr);

            ContextManager::SetConstantBuffer(2, m_SurfaceMaterialBufferPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

            for (auto Component : DataMeshComponents)
            {
                Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

                if (pDtComponent->IsActiveAndUsable() == false) continue;

                CMesh* pMesh = static_cast<CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------

                if (pMesh->GetNumberOfLODs() == 0) continue;

                CSurfacePtr SurfacePtr = pMesh->GetLOD(0)->GetSurface();

                const Gfx::CMaterial* pMaterial = SurfacePtr->GetMaterial();

                if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pDtMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    pMaterial = static_cast<const Gfx::CMaterial*>(pDtMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }

                if (!pMaterial->HasRefraction()) continue;

                for (int Index = 0; Index < pMaterial->GetTextureSetPS()->GetNumberOfTextures(); ++Index)
                {
                    ContextManager::SetSampler(Index, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

                    ContextManager::SetTexture(Index, pMaterial->GetTextureSetPS()->GetTexture(Index));
                }

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

                BufferManager::UploadBufferData(m_PerMeshConstantBufferPtr, &pDtComponent->GetHostEntity()->GetTransformationFacet()->GetWorldMatrix());

                BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &pMaterial->GetMaterialAttributes());

                // -----------------------------------------------------------------------------

                ContextManager::SetShaderVS(SurfacePtr->GetMVPShaderVS());

                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
            }

            Performance::EndEvent();

            // -----------------------------------------------------------------------------

            Performance::BeginEvent("Background");

            TargetSetManager::ClearTargetSet(m_BackgroundTargetSetPtr);

            ContextManager::SetTargetSet(m_BackgroundTargetSetPtr);

            ContextManager::SetViewPortSet(m_ViewportSetPtr);

            ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));

            ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

            ContextManager::SetConstantBuffer(0, m_PerLightConstantBufferPtr);

            ContextManager::SetConstantBuffer(1, m_PerMeshConstantBufferPtr);

            ContextManager::SetConstantBuffer(2, m_SurfaceMaterialBufferPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderPS(m_NormalPSPtr);

            for (auto Component : DataMeshComponents)
            {
                Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

                if (pDtComponent->IsActiveAndUsable() == false) continue;

                CMesh* pMesh = static_cast<CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------

                if (pMesh->GetNumberOfLODs() == 0) continue;

                CSurfacePtr SurfacePtr = pMesh->GetLOD(0)->GetSurface();

                const Gfx::CMaterial* pMaterial = SurfacePtr->GetMaterial();

                if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pDtMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    pMaterial = static_cast<const Gfx::CMaterial*>(pDtMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }

                if (pMaterial->HasRefraction()) continue;

                // -----------------------------------------------------------------------------

                BufferManager::UploadBufferData(m_PerMeshConstantBufferPtr, &pDtComponent->GetHostEntity()->GetTransformationFacet()->GetWorldMatrix());

                BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &pMaterial->GetMaterialAttributes());

                // -----------------------------------------------------------------------------

                ContextManager::SetShaderVS(SurfacePtr->GetMVPShaderVS());

                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
            }

            Performance::EndEvent();

            // -----------------------------------------------------------------------------

            Performance::BeginEvent("Photon Emission");

            TargetSetManager::ClearTargetSet(m_PhotonEmissionTargetSetPtr);

            ContextManager::SetTargetSet(m_PhotonEmissionTargetSetPtr);

            ContextManager::SetViewPortSet(m_ViewportSetPtr);

            ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));

            ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

            ContextManager::SetConstantBuffer(0, m_PerLightConstantBufferPtr);

            ContextManager::SetConstantBuffer(1, m_PerMeshConstantBufferPtr);

            ContextManager::SetConstantBuffer(3, m_LightPropertiesBufferPtr);

            ContextManager::SetConstantBuffer(4, m_CausticSettingsBufferPtr);

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(0, m_RefractiveNormalTexturePtr);
            
            ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(1, m_RefractiveDepthTexturePtr);

            ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(2, m_BackgroundDepthTexturePtr);

            ContextManager::SetShaderPS(m_PhotonEmissionPSPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            for (auto Component : DataMeshComponents)
            {
                Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

                if (pDtComponent->IsActiveAndUsable() == false) continue;

                CMesh* pMesh = static_cast<CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------

                if (pMesh->GetNumberOfLODs() == 0) continue;

                CSurfacePtr SurfacePtr = pMesh->GetLOD(0)->GetSurface();

                const Gfx::CMaterial* pMaterial = SurfacePtr->GetMaterial();

                if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pDtMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    pMaterial = static_cast<const Gfx::CMaterial*>(pDtMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }

                if (!pMaterial->HasRefraction()) continue;

                // -----------------------------------------------------------------------------

                BufferManager::UploadBufferData(m_PerMeshConstantBufferPtr, &pDtComponent->GetHostEntity()->GetTransformationFacet()->GetWorldMatrix());

                // -----------------------------------------------------------------------------

                float Near              = pPointLight->GetCamera()->GetNear();
                float Far               = pPointLight->GetCamera()->GetFar();
                float IndexOfRefraction = pMaterial->GetMaterialRefractionAttributes().m_IndexOfRefraction;

                SCausticSettingsBuffer CausticSettingsBuffer;

                CausticSettingsBuffer.m_RefractionIndices  = glm::vec4(1.0f / IndexOfRefraction, 1.0f / (IndexOfRefraction * IndexOfRefraction), IndexOfRefraction, IndexOfRefraction * IndexOfRefraction);
                CausticSettingsBuffer.m_DepthLinearization = glm::vec4(Near * Far, Far - Near, Far + Near, Far);

                BufferManager::UploadBufferData(m_CausticSettingsBufferPtr, &CausticSettingsBuffer);

                // -----------------------------------------------------------------------------

                ContextManager::SetShaderVS(SurfacePtr->GetMVPShaderVS());

                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
            }

            Performance::EndEvent();

            Performance::BeginEvent("Gathering");

            TargetSetManager::ClearTargetSet(m_PhotonGatheringTargetSetPtr);

            ContextManager::SetTargetSet(m_PhotonGatheringTargetSetPtr);

            ContextManager::SetViewPortSet(m_ViewportSetPtr);

            ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

            ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AdditionBlend));

            ContextManager::SetConstantBuffer(0, m_PerLightConstantBufferPtr);

            ContextManager::SetShaderVS(m_PhotonGatheringVSPtr);

            ContextManager::SetShaderPS(m_PhotonGatheringPSPtr);

            ContextManager::SetTopology(STopology::PointList);

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

            ContextManager::SetTexture(0, m_PhotonLocationTexturePtr);

            ContextManager::Draw(s_CausticMapSize * s_CausticMapSize, 0);

            Performance::EndEvent();

            Performance::BeginEvent("Apply caustic");

            ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());

            ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

            ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

            ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AdditionBlend));

            ContextManager::SetShaderVS(m_FullscreenVSPtr);

            ContextManager::SetShaderPS(m_PhotonApplyPSPtr);

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

            ContextManager::SetConstantBuffer(1, m_LightPropertiesBufferPtr);

            ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

            ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

            ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

            ContextManager::SetTexture(1, m_PhotonGatheringTexturePtr);

            ContextManager::Draw(3, 0);

            Performance::EndEvent();
        }

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetConstantBuffer(4);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

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
