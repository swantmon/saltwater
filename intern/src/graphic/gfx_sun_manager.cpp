
#include "graphic/gfx_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "base/base_include_glm.h"

#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_mesh_component.h"
#include "data/data_sun_component.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <map>

using namespace Gfx;

namespace
{
    class CGfxSunManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxSunManager)
        
    public:
        
        CGfxSunManager();
        ~CGfxSunManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Update();

    private:

        struct SPerLightConstantBuffer
        {
            glm::mat4 vs_ViewProjectionMatrix;
        };

        struct SPerDrawCallConstantBuffer
        {
            glm::mat4 m_ModelMatrix;
        };

        class CInternSunComponent : public CSun
        {
        public:

            CInternSunComponent();
            ~CInternSunComponent();

        public:

            CRenderContextPtr m_RenderContextPtr;

        private:

            friend class CGfxSunManager;
        };

    private:

        typedef Base::CManagedPool<CInternSunComponent, 4, 0> CSuns;

    private:

        CSuns m_Suns;

        CShaderPtr m_ShadowShaderVSPtr;
        CShaderPtr m_ShadowSMShaderPSPtr;
        
        CBufferSetPtr m_LightCameraVSBufferPtr;
        
    private:

        void OnDirtyComponent(Base::IComponent* _pComponent);

        void CreateSM(unsigned int _Size, CInternSunComponent* _pInternLight);

        void RenderShadows(CInternSunComponent* _pInternLight);
    };
} // namespace

namespace
{
    CGfxSunManager::CInternSunComponent::CInternSunComponent()
        : CSun              ()
        , m_RenderContextPtr()
    {
        
    }

    // -----------------------------------------------------------------------------

    CGfxSunManager::CInternSunComponent::~CInternSunComponent()
    {
        m_RenderContextPtr = 0;
    }
} // namespace 

namespace
{
    CGfxSunManager::CGfxSunManager()
        : m_Suns                  ()
        , m_ShadowShaderVSPtr     ()
        , m_ShadowSMShaderPSPtr   ()
        , m_LightCameraVSBufferPtr()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxSunManager::~CGfxSunManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSunManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_ShadowShaderVSPtr   = ShaderManager::CompileVS("vs_vm_pnx0.glsl", "main");
        m_ShadowSMShaderPSPtr = ShaderManager::CompilePS("fs_shadow.glsl", "SM");

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerLightConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerLightConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerDrawCallConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        m_LightCameraVSBufferPtr = BufferManager::CreateBufferSet(PerLightConstantBuffer, PerDrawCallConstantBuffer);
        
        // -----------------------------------------------------------------------------
        // On dirty stuff
        // -----------------------------------------------------------------------------
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(BASE_DIRTY_COMPONENT_METHOD(&CGfxSunManager::OnDirtyComponent));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSunManager::OnExit()
    {
        m_Suns.Clear();

        m_ShadowShaderVSPtr      = 0;
        m_ShadowSMShaderPSPtr    = 0;
        m_LightCameraVSBufferPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxSunManager::Update()
    {
        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSunComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CSunComponent* pDtComponent = static_cast<Dt::CSunComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            CInternSunComponent* pGfxSunFacet = static_cast<CInternSunComponent*>(pDtComponent->GetFacet(Dt::CSunComponent::Graphic));

            assert(pGfxSunFacet != nullptr);

            if (pDtComponent->GetRefreshMode() == Dt::CSunComponent::Dynamic || pGfxSunFacet->GetTimeStamp() >= Core::Time::GetNumberOfFrame())
            {
                // -----------------------------------------------------------------------------
                // Calculate near and far plane
                // -----------------------------------------------------------------------------
                float Radius = 30.0f;
                float Near   = 1.0f;
                float Far    = Radius * 2.0f;

                pGfxSunFacet->m_RenderContextPtr->GetCamera()->SetOrthographic(-Radius, Radius, -Radius, Radius, Near, Far);

                // -----------------------------------------------------------------------------
                // Set view
                // -----------------------------------------------------------------------------
                glm::vec3 SunPosition    = pDtComponent->GetHostEntity()->GetWorldPosition();
                glm::vec3 SunRotation    = pDtComponent->GetDirection();
                glm::mat3 RotationMatrix = glm::lookAtRH(SunPosition, SunPosition + SunRotation, glm::vec3(0.0f, 0.0f, 1.0f));

                Gfx::CViewPtr ShadowViewPtr = pGfxSunFacet->m_RenderContextPtr->GetCamera()->GetView();

                ShadowViewPtr->SetPosition(SunPosition);
                ShadowViewPtr->SetRotationMatrix(RotationMatrix);

                ShadowViewPtr->Update();

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                RenderShadows(pGfxSunFacet);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSunManager::OnDirtyComponent(Base::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CSunComponent>()) return;

        Dt::CSunComponent* pSunComponent = static_cast<Dt::CSunComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pSunComponent->GetDirtyFlags();

        if ((DirtyFlags & Dt::CSunComponent::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            CInternSunComponent* pGfxSunFacet = m_Suns.Allocate();

            // -----------------------------------------------------------------------------
            // Set shadow data
            // -----------------------------------------------------------------------------
            CreateSM(2048, pGfxSunFacet);

            // -----------------------------------------------------------------------------
            // Set variables
            // -----------------------------------------------------------------------------
            pGfxSunFacet->m_CameraPtr = pGfxSunFacet->m_RenderContextPtr->GetCamera();

            // -----------------------------------------------------------------------------
            // Set dirty
            // -----------------------------------------------------------------------------
            pGfxSunFacet->m_TimeStamp = Core::Time::GetNumberOfFrame() + 1;

            // -----------------------------------------------------------------------------
            // Link
            // -----------------------------------------------------------------------------
            pSunComponent->SetFacet(Dt::CSunComponent::Graphic, pGfxSunFacet);
        }
        else
        {
            CInternSunComponent* pGfxSunFacet = static_cast<CInternSunComponent*>(pSunComponent->GetFacet(Dt::CSunComponent::Graphic));

            assert(pGfxSunFacet != nullptr);

            pGfxSunFacet->m_TimeStamp = Core::Time::GetNumberOfFrame() + 1;
        }
    }

    // -----------------------------------------------------------------------------
    
    void CGfxSunManager::CreateSM(unsigned int _Size, CInternSunComponent* _pInternLight)
    {
        unsigned int NumberOfShadowMapPixel = _Size;
        
        // -----------------------------------------------------------------------------
        // Create texture for shadow mapping
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = NumberOfShadowMapPixel;
        RendertargetDescriptor.m_NumberOfPixelsV  = NumberOfShadowMapPixel;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Binding          = CTexture::DepthStencilTarget | CTexture::RenderTarget;
        RendertargetDescriptor.m_Format           = CTexture::R32_FLOAT;
        
        _pInternLight->m_TextureSMPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth only

		TextureManager::SetTextureLabel(_pInternLight->m_TextureSMPtr, "Sun: Shadowmap");
        
        // -----------------------------------------------------------------------------
        // Create target set for shadow mapping
        // -----------------------------------------------------------------------------
        CTargetSetPtr ShadowTargetSetPtr = TargetSetManager::CreateTargetSet(_pInternLight->m_TextureSMPtr);

		TargetSetManager::SetTargetSetLabel(ShadowTargetSetPtr, "Sun: Shadowmap");
        
        // -----------------------------------------------------------------------------
        // Create view and camera
        // -----------------------------------------------------------------------------
        CViewPtr   ShadowViewPtr = ViewManager::CreateView();
        
        // -----------------------------------------------------------------------------
        // Create view port
        // -----------------------------------------------------------------------------
        SViewPortDescriptor ViewPortDesc;
        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(NumberOfShadowMapPixel);
        ViewPortDesc.m_Height   = static_cast<float>(NumberOfShadowMapPixel);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr ShadowViewPort = ViewManager::CreateViewPort(ViewPortDesc);
        
        // -----------------------------------------------------------------------------
        // Create render context with all informations
        // -----------------------------------------------------------------------------
        CCameraPtr      CameraPtr      = ViewManager::CreateCamera(ShadowViewPtr);
        CViewPortSetPtr ViewPortSetPtr = ViewManager::CreateViewPortSet(ShadowViewPort);
        CRenderStatePtr RenderStatePtr = StateManager::GetRenderState(0);
        CTargetSetPtr   TargetSetPtr   = ShadowTargetSetPtr;
        
        _pInternLight->m_RenderContextPtr = ContextManager::CreateRenderContext();
        
        _pInternLight->m_RenderContextPtr->SetCamera(CameraPtr);
        _pInternLight->m_RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        _pInternLight->m_RenderContextPtr->SetTargetSet(TargetSetPtr);
        _pInternLight->m_RenderContextPtr->SetRenderState(RenderStatePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxSunManager::RenderShadows(CInternSunComponent* _pInternLight)
    {
        Performance::BeginEvent("Sun Shadows");

        // -----------------------------------------------------------------------------
        // Prepare shadow
        // -----------------------------------------------------------------------------
        TargetSetManager::ClearTargetSet(_pInternLight->m_RenderContextPtr->GetTargetSet());
            
        // -----------------------------------------------------------------------------
        // Set light as render target
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(_pInternLight->m_RenderContextPtr);
            
        // -----------------------------------------------------------------------------
        // Set shader
        // -----------------------------------------------------------------------------
        ContextManager::SetShaderVS(m_ShadowShaderVSPtr);
            
        ContextManager::SetShaderPS(m_ShadowSMShaderPSPtr);
            
        // -----------------------------------------------------------------------------
        // Set constant buffer
        // -----------------------------------------------------------------------------
        ContextManager::SetConstantBuffer(0, m_LightCameraVSBufferPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(1, m_LightCameraVSBufferPtr->GetBuffer(1));
            
        // -----------------------------------------------------------------------------
        // Upload data light view projection matrix
        // -----------------------------------------------------------------------------
        SPerLightConstantBuffer ViewBuffer;

        ViewBuffer.vs_ViewProjectionMatrix = _pInternLight->m_RenderContextPtr->GetCamera()->GetViewProjectionMatrix();
            
        BufferManager::UploadBufferData(m_LightCameraVSBufferPtr->GetBuffer(0), &ViewBuffer);
            
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

        for (auto Component : DataMeshComponents)
        {
            Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            CMesh* pGfxComponent = static_cast<CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

            CMeshPtr MeshPtr = pGfxComponent;

            // -----------------------------------------------------------------------------
            // Upload model matrix to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBuffer ModelBuffer;

            ModelBuffer.m_ModelMatrix = pDtComponent->GetHostEntity()->GetTransformationFacet()->GetWorldMatrix();

            BufferManager::UploadBufferData(m_LightCameraVSBufferPtr->GetBuffer(1), &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render every surface of this entity
            // -----------------------------------------------------------------------------
            CSurfacePtr SurfacePtr = MeshPtr->GetLOD(0)->GetSurface();

            if (SurfacePtr == nullptr)
            {
                continue;
            }

            // -----------------------------------------------------------------------------
            // Get input layout from optimal shader
            // -----------------------------------------------------------------------------
            assert(SurfacePtr->GetKey().m_HasPosition);

            CInputLayoutPtr LayoutPtr = SurfacePtr->GetShaderVS()->GetInputLayout();

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(LayoutPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetTopology();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBuffer();
        }

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetShaderVS();
            
        ContextManager::ResetShaderPS();
            
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }
} // namespace

namespace Gfx
{
namespace SunManager
{
    void OnStart()
    {
        CGfxSunManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxSunManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxSunManager::GetInstance().Update();
    }
} // namespace SunManager
} // namespace Gfx