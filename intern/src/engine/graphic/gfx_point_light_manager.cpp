
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_time.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_point_light_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_material.h"
#include "engine/graphic/gfx_mesh.h"
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
    class CGfxPointLightManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPointLightManager)

    public:

        CGfxPointLightManager();
        ~CGfxPointLightManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

    private:

        struct SPerLightConstantBuffer
        {
            glm::mat4 vs_ProjectionMatrix;
            glm::mat4 vs_ViewMatrix;
        };

        struct SPerDrawCallConstantBuffer
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SPunctualLightProperties
        {
            glm::vec4 m_LightPosition;
            glm::vec4 m_LightDirection;
            glm::vec4 m_LightColor;
            glm::vec4 m_LightSettings; // InvSqrAttenuationRadius, AngleScale, AngleOffset, Has shadows
        };

        class CInternObject : public CPointLight
        {
        public:

            CInternObject();
            ~CInternObject();

        public:

            CRenderContextPtr m_RenderContextPtr;
            Dt::CPointLightComponent::EShadowType m_CurrentShadowType;

        private:

            friend class CGfxPointLightManager;
        };

    private:

        typedef Base::CManagedPool<CInternObject, 32, 0> CPointLights;

    private:

        CPointLights m_PointLights;
        CShaderPtr m_ShadowSMShaderPSPtr;
        CShaderPtr m_ShadowRSMShaderPSPtr;
        CShaderPtr m_ShadowRSMTexShaderPSPtr;
        CBufferSetPtr m_LightCameraVSBufferPtr;
        CBufferSetPtr m_RSMPSBuffer;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void CreateRSM(unsigned int _Size, CInternObject* _pInternLight);

        void CreateSM(unsigned int _Size, CInternObject* _pInternLight);

        void RenderShadows(CInternObject& _rInternLight, const Dt::CPointLightComponent* _pDtPointLight, const glm::vec3& _rLightPosition);
    };
} // namespace 

namespace 
{
    CGfxPointLightManager::CInternObject::CInternObject()
        : CPointLight        ()
        , m_RenderContextPtr ()
        , m_CurrentShadowType(Dt::CPointLightComponent::NoShadows)
    {

    }

    // -----------------------------------------------------------------------------

    CGfxPointLightManager::CInternObject::~CInternObject()
    {
        m_RenderContextPtr = 0;
    }
} // namespace 

namespace 
{
    CGfxPointLightManager::CGfxPointLightManager()
        : m_PointLights            ()
        , m_ShadowSMShaderPSPtr    ()
        , m_ShadowRSMShaderPSPtr   ()
        , m_ShadowRSMTexShaderPSPtr()
        , m_LightCameraVSBufferPtr ()
        , m_RSMPSBuffer            ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxPointLightManager::~CGfxPointLightManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_ShadowSMShaderPSPtr     = ShaderManager::CompilePS("shadow/fs_shadow.glsl", "SM");
        m_ShadowRSMShaderPSPtr    = ShaderManager::CompilePS("shadow/fs_shadow.glsl", "RSM_COLOR");
        m_ShadowRSMTexShaderPSPtr = ShaderManager::CompilePS("shadow/fs_shadow.glsl", "RSM_TEX");

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

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr MaterialBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPunctualLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PointLightBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        m_RSMPSBuffer            = BufferManager::CreateBufferSet(MaterialBuffer, PointLightBufferPtr);

        m_LightCameraVSBufferPtr = BufferManager::CreateBufferSet(PerLightConstantBuffer, PerDrawCallConstantBuffer);
        
        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CGfxPointLightManager::OnDirtyComponent));

        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxPointLightManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::OnExit()
    {
        m_PointLights.Clear();

        m_ShadowSMShaderPSPtr     = 0;
        m_ShadowRSMShaderPSPtr    = 0;
        m_ShadowRSMTexShaderPSPtr = 0;
        m_LightCameraVSBufferPtr  = 0;
        m_RSMPSBuffer             = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::Update()
    {
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CPointLightComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CPointLightComponent* pDtComponent = static_cast<Dt::CPointLightComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            CInternObject* pGfxPointLight = static_cast<CInternObject*>(pDtComponent->GetFacet(Dt::CPointLightComponent::Graphic));

            if (pDtComponent->GetRefreshMode() == Dt::CPointLightComponent::Dynamic)
            {
                // -----------------------------------------------------------------------------
                // Update views
                // -----------------------------------------------------------------------------
                Gfx::CViewPtr   ShadowViewPtr   = pGfxPointLight->m_RenderContextPtr->GetCamera()->GetView();
                Gfx::CCameraPtr ShadowCameraPtr = pGfxPointLight->m_RenderContextPtr->GetCamera();

                glm::vec3 LightPosition  = pDtComponent->GetHostEntity()->GetWorldPosition();
                glm::vec3 LightDirection = pDtComponent->GetDirection();

                // -----------------------------------------------------------------------------
                // Set view
                // -----------------------------------------------------------------------------
                glm::mat3 RotationMatrix = glm::mat3(1.0f);

                RotationMatrix = glm::lookAt(LightPosition, LightPosition + LightDirection, glm::vec3(0.0f, 0.0f, 1.0f));

                ShadowViewPtr->SetPosition(LightPosition);
                ShadowViewPtr->SetRotationMatrix(glm::transpose(RotationMatrix));

                // -----------------------------------------------------------------------------
                // Calculate near and far plane
                // -----------------------------------------------------------------------------
                float Near = 0.1f;
                float Far = pDtComponent->GetAttenuationRadius() + Near;

                // -----------------------------------------------------------------------------
                // Set matrix
                // -----------------------------------------------------------------------------
                ShadowCameraPtr->SetFieldOfView(glm::degrees(pDtComponent->GetOuterConeAngle()), 1.0f, Near, Far);

                ShadowViewPtr->Update();

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                RenderShadows(*pGfxPointLight, pDtComponent, LightPosition);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if (_pEntity->GetDirtyFlags() != Dt::CEntity::DirtyMove) return;

        auto ComponentFacet = _pEntity->GetComponentFacet();

        if (!ComponentFacet->HasComponent<Dt::CPointLightComponent>()) return;

        auto AreaLightComponents = ComponentFacet->GetComponents();

        for (auto pComponent : AreaLightComponents)
        {
            if (pComponent->GetTypeID() == Base::CTypeInfo::GetTypeID<Dt::CPointLightComponent>())
            {
                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CPointLightComponent::DirtyInfo);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CPointLightComponent>()) return;

        Dt::CPointLightComponent* pPointLightComponent = static_cast<Dt::CPointLightComponent*>(_pComponent);

        CInternObject* pGfxPointLightFacet = 0;
        Dt::CPointLightComponent::EShadowType ShadowType;
        unsigned int ShadowmapSizes[Dt::CPointLightComponent::NumberOfQualities] = { 256, 512, 1024, 2048 };
        unsigned int ShadowmapSize = 0;

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pPointLightComponent->GetDirtyFlags();

        if ((DirtyFlags & Dt::CPointLightComponent::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            pGfxPointLightFacet = m_PointLights.Allocate();

            // -----------------------------------------------------------------------------
            // Set shadow data
            // -----------------------------------------------------------------------------
            ShadowmapSize = ShadowmapSizes[pPointLightComponent->GetShadowQuality()];

            ShadowType = pPointLightComponent->GetShadowType();

            switch (ShadowType)
            {
            case Dt::CPointLightComponent::HardShadows:        CreateSM(ShadowmapSize, pGfxPointLightFacet); break;
            case Dt::CPointLightComponent::GlobalIllumination: CreateRSM(ShadowmapSize, pGfxPointLightFacet); break;
            }

            // -----------------------------------------------------------------------------
            // Set variables
            // -----------------------------------------------------------------------------
            pGfxPointLightFacet->m_ShadowmapSize = ShadowmapSize;

            pGfxPointLightFacet->m_CurrentShadowType = ShadowType;

            // -----------------------------------------------------------------------------
            // Link
            // -----------------------------------------------------------------------------
            pPointLightComponent->SetFacet(Dt::CPointLightComponent::Graphic, pGfxPointLightFacet);
        }
        else if ((DirtyFlags & Dt::CPointLightComponent::DirtyInfo) != 0)
        {
            pGfxPointLightFacet = static_cast<CInternObject*>(pPointLightComponent->GetFacet(Dt::CPointLightComponent::Graphic));

            assert(pGfxPointLightFacet);

            ShadowmapSize = ShadowmapSizes[pPointLightComponent->GetShadowQuality()];

            ShadowType = pPointLightComponent->GetShadowType();

            if (ShadowmapSize != pGfxPointLightFacet->m_ShadowmapSize || ShadowType != pGfxPointLightFacet->m_CurrentShadowType)
            {
                pGfxPointLightFacet->m_ShadowmapSize = ShadowmapSize;

                pGfxPointLightFacet->m_CurrentShadowType = ShadowType;

                switch (ShadowType)
                {
                case Dt::CPointLightComponent::HardShadows:        CreateSM(ShadowmapSize, pGfxPointLightFacet); break;
                case Dt::CPointLightComponent::GlobalIllumination: CreateRSM(ShadowmapSize, pGfxPointLightFacet); break;
                }
            }
        }
        else
        {
            pGfxPointLightFacet = static_cast<CInternObject*>(pPointLightComponent->GetFacet(Dt::CPointLightComponent::Graphic));
        }
        
        assert(pGfxPointLightFacet);

        // -----------------------------------------------------------------------------
        // Update views
        // -----------------------------------------------------------------------------
        Gfx::CViewPtr   ShadowViewPtr   = pGfxPointLightFacet->m_RenderContextPtr->GetCamera()->GetView();
        Gfx::CCameraPtr ShadowCameraPtr = pGfxPointLightFacet->m_RenderContextPtr->GetCamera();

        glm::vec3 LightPosition  = pPointLightComponent->GetHostEntity()->GetWorldPosition();
        glm::vec3 LightDirection = pPointLightComponent->GetDirection();

        // -----------------------------------------------------------------------------
        // Set view
        // -----------------------------------------------------------------------------
        glm::mat3 RotationMatrix = glm::mat3(1.0f);

        RotationMatrix = glm::lookAt(LightPosition, LightPosition + LightDirection, glm::vec3(0.0f, 0.0f, 1.0f));

        ShadowViewPtr->SetPosition(LightPosition);
        ShadowViewPtr->SetRotationMatrix(glm::transpose(RotationMatrix));

        // -----------------------------------------------------------------------------
        // Calculate near and far plane
        // -----------------------------------------------------------------------------
        float Near = 0.1f;
        float Far = pPointLightComponent->GetAttenuationRadius() + Near;

        // -----------------------------------------------------------------------------
        // Set matrix
        // -----------------------------------------------------------------------------
        ShadowCameraPtr->SetFieldOfView(glm::degrees(pPointLightComponent->GetOuterConeAngle()), 1.0f, Near, Far);

        ShadowViewPtr->Update();

        // -----------------------------------------------------------------------------
        // Render shadows
        // -----------------------------------------------------------------------------
        RenderShadows(*pGfxPointLightFacet, pPointLightComponent, LightPosition);

        // -----------------------------------------------------------------------------
        // Set time
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGfxPointLightFacet->m_TimeStamp = FrameTime;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxPointLightManager::CreateRSM(unsigned int _Size, CInternObject* _pInternLight)
    {
        unsigned int NumberOfShadowMapPixel = _Size;
        
        // -----------------------------------------------------------------------------
        // Create texture for reflective shadow mapping
        // 0 = Position
        // 1 = Normal
        // 2 = Flux
        // 3 = Depth
        // -----------------------------------------------------------------------------
        CTexturePtr ShadowRenderbuffer[4];
        
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
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        ShadowRenderbuffer[0] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Position
        
        RendertargetDescriptor.m_Binding = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format  = CTexture::R16G16B16A16_FLOAT;
        
        ShadowRenderbuffer[1] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Normal
        
        RendertargetDescriptor.m_Binding = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format  = CTexture::R16G16B16A16_FLOAT;
        
        ShadowRenderbuffer[2] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Flux
        
        RendertargetDescriptor.m_Binding = CTexture::DepthStencilTarget | CTexture::RenderTarget;
        RendertargetDescriptor.m_Format  = CTexture::R32_FLOAT;
        
        ShadowRenderbuffer[3] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth
        
        _pInternLight->m_TextureSMPtr = TextureManager::CreateTextureSet(ShadowRenderbuffer[3]);

        _pInternLight->m_TextureRSMPtr = TextureManager::CreateTextureSet(ShadowRenderbuffer, 4);
        
        // -----------------------------------------------------------------------------
        // Create target set for shadow mapping
        // -----------------------------------------------------------------------------
        CTargetSetPtr ShadowTargetSetPtr = TargetSetManager::CreateTargetSet(ShadowRenderbuffer, 4);
        
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
        
        assert(_pInternLight->m_RenderContextPtr.IsValid());

        _pInternLight->m_RenderContextPtr->SetCamera(CameraPtr);
        _pInternLight->m_RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        _pInternLight->m_RenderContextPtr->SetTargetSet(TargetSetPtr);
        _pInternLight->m_RenderContextPtr->SetRenderState(RenderStatePtr);

        // -----------------------------------------------------------------------------
        // Save camera
        // -----------------------------------------------------------------------------
        _pInternLight->m_CameraPtr = CameraPtr;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxPointLightManager::CreateSM(unsigned int _Size, CInternObject* _pInternLight)
    {
        unsigned int NumberOfShadowMapPixel = _Size;
        
        // -----------------------------------------------------------------------------
        // Create texture for shadow mapping
        // -----------------------------------------------------------------------------
        CTexturePtr ShadowRenderbuffer[1];
        
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
        
        ShadowRenderbuffer[0] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth only
        
        _pInternLight->m_TextureSMPtr  = TextureManager::CreateTextureSet(ShadowRenderbuffer, 1);

        _pInternLight->m_TextureRSMPtr = 0;
        
        // -----------------------------------------------------------------------------
        // Create target set for shadow mapping
        // -----------------------------------------------------------------------------
        CTargetSetPtr ShadowTargetSetPtr = TargetSetManager::CreateTargetSet(ShadowRenderbuffer, 1);
        
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

        // -----------------------------------------------------------------------------
        // Save camera
        // -----------------------------------------------------------------------------
        _pInternLight->m_CameraPtr = CameraPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::RenderShadows(CInternObject& _rInternLight, const Dt::CPointLightComponent* _pDtPointLight, const glm::vec3& _rLightPosition)
    {
        if (_rInternLight.m_CurrentShadowType == Dt::CPointLightComponent::NoShadows) return;

        Performance::BeginEvent("Point Light Shadows");
        
        // -----------------------------------------------------------------------------
        // Prepare shadow
        // -----------------------------------------------------------------------------
        TargetSetManager::ClearTargetSet(_rInternLight.m_RenderContextPtr->GetTargetSet());
            
        // -----------------------------------------------------------------------------
        // Set light as render target
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(_rInternLight.m_RenderContextPtr);
   
        // -----------------------------------------------------------------------------
        // Upload data light view projection matrix
        // -----------------------------------------------------------------------------
        SPerLightConstantBuffer ViewBuffer;
            
        ViewBuffer.vs_ProjectionMatrix = _rInternLight.m_RenderContextPtr->GetCamera()->GetProjectionMatrix();
        ViewBuffer.vs_ViewMatrix       = _rInternLight.m_RenderContextPtr->GetCamera()->GetView()->GetViewMatrix();
            
        BufferManager::UploadBufferData(m_LightCameraVSBufferPtr->GetBuffer(0), &ViewBuffer);
            
        // -----------------------------------------------------------------------------
        // Iterate throw every component inside this map
        // -----------------------------------------------------------------------------
        auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

        for (auto Component : DataMeshComponents)
        {
            Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            CMesh* pMesh = static_cast<CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

            // -----------------------------------------------------------------------------
            // Upload model matrix to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBuffer ModelBuffer;

            ModelBuffer.m_ModelMatrix = pDtComponent->GetHostEntity()->GetTransformationFacet()->GetWorldMatrix();

            BufferManager::UploadBufferData(m_LightCameraVSBufferPtr->GetBuffer(1), &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render surface of this entity
            // -----------------------------------------------------------------------------
            CSurfacePtr SurfacePtr = pMesh->GetLOD(0)->GetSurface();

            // -----------------------------------------------------------------------------
            // Set material
            // -----------------------------------------------------------------------------
            const Gfx::CMaterial* pMaterial = SurfacePtr->GetMaterial();

            if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
            {
                auto pDtMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                pMaterial = static_cast<const Gfx::CMaterial*>(pDtMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
            }

            // -----------------------------------------------------------------------------
            // Set shader + buffer
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetMVPShaderVS());

            ContextManager::SetConstantBuffer(0, m_LightCameraVSBufferPtr->GetBuffer(0));

            ContextManager::SetConstantBuffer(1, m_LightCameraVSBufferPtr->GetBuffer(1));

            if (_rInternLight.m_CurrentShadowType == Dt::CPointLightComponent::GlobalIllumination)
            {
                if (pMaterial->GetKey().m_HasDiffuseTex)
                {
                    ContextManager::SetShaderPS(m_ShadowRSMTexShaderPSPtr);

                    for (unsigned int IndexOfTexture = 0; IndexOfTexture < pMaterial->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
                    {
                        ContextManager::SetSampler(IndexOfTexture, pMaterial->GetSamplerSetPS()->GetSampler(IndexOfTexture));

                        ContextManager::SetTexture(IndexOfTexture, pMaterial->GetTextureSetPS()->GetTexture(IndexOfTexture));
                    }
                }
                else
                {
                    ContextManager::SetShaderPS(m_ShadowRSMShaderPSPtr);
                }

                BufferManager::UploadBufferData(m_RSMPSBuffer->GetBuffer(0), &pMaterial->GetMaterialAttributes());

                // -----------------------------------------------------------------------------

                SPunctualLightProperties PunctualLightProperties;

                float InvSqrAttenuationRadius = _pDtPointLight->GetReciprocalSquaredAttenuationRadius();
                float AngleScale = _pDtPointLight->GetAngleScale();
                float AngleOffset = _pDtPointLight->GetAngleOffset();

                PunctualLightProperties.m_LightPosition  = glm::vec4(_rLightPosition, 1.0f);
                PunctualLightProperties.m_LightDirection = glm::normalize(glm::vec4(_pDtPointLight->GetDirection(), 0.0f));
                PunctualLightProperties.m_LightColor     = glm::vec4(_pDtPointLight->GetLightness(), 1.0f);
                PunctualLightProperties.m_LightSettings  = glm::vec4(InvSqrAttenuationRadius, AngleScale, AngleOffset, 0.0f);

                BufferManager::UploadBufferData(m_RSMPSBuffer->GetBuffer(1), &PunctualLightProperties);

                ContextManager::SetConstantBuffer(2, m_RSMPSBuffer->GetBuffer(0));

                ContextManager::SetConstantBuffer(3, m_RSMPSBuffer->GetBuffer(1));
            }
            else
            {
                ContextManager::SetShaderPS(m_ShadowSMShaderPSPtr);
            }

            // -----------------------------------------------------------------------------
            // Get input layout from optimal shader
            // -----------------------------------------------------------------------------
            assert(SurfacePtr->GetKey().m_HasPosition);

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetTopology();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBuffer();
        }

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetShaderVS();
            
        ContextManager::ResetShaderPS();
            
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }
} // namespace 

namespace Gfx
{
namespace PointLightManager
{
    void OnStart()
    {
        CGfxPointLightManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxPointLightManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxPointLightManager::GetInstance().Update();
    }
} // namespace PointLightManager
} // namespace Gfx