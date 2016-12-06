
#include "graphic/gfx_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"
#include "data/data_point_light_facet.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_facet.h"
#include "graphic/gfx_point_light_manager.h"
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
            Base::Float4x4 vs_ViewProjectionMatrix;
        };

        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SPunctualLightProperties
        {
            Base::Float4 m_LightPosition;
            Base::Float4 m_LightDirection;
            Base::Float4 m_LightColor;
            Base::Float4 m_LightSettings; // InvSqrAttenuationRadius, AngleScale, AngleOffset, Has shadows
        };

        class CInternPointLightFacet : public CPointLightFacet
        {
        public:

            CInternPointLightFacet();
            ~CInternPointLightFacet();

        public:

            CRenderContextPtr m_RenderContextPtr;
            Dt::CPointLightFacet::EShadowType m_CurrentShadowType;

        private:

            friend class CGfxPointLightManager;
        };

    private:

        typedef Base::CPool<CInternPointLightFacet, 64> CPointLightFacets;

    private:

        CShaderPtr m_ShadowShaderVSPtr;
        CShaderPtr m_ShadowSMShaderPSPtr;
        CShaderPtr m_ShadowRSMShaderPSPtr;
        CShaderPtr m_ShadowRSMTexShaderPSPtr;
        CBufferSetPtr m_LightCameraVSBufferPtr;
        CBufferSetPtr m_MainVSBufferPtr;
        CBufferSetPtr m_RSMPSBuffer;

        CPointLightFacets m_PointLightFacets;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternPointLightFacet& AllocatePointLightFacet();

        void CreateRSM(unsigned int _Size, CInternPointLightFacet& _rInternLight);

        void CreateSM(unsigned int _Size, CInternPointLightFacet& _rInternLight);

        void RenderShadows(CInternPointLightFacet& _rInternLight, const Dt::CPointLightFacet* _pDtPointLight, const Base::Float3& _rLightPosition);
    };
} // namespace 

namespace 
{
    CGfxPointLightManager::CInternPointLightFacet::CInternPointLightFacet()
        : CPointLightFacet  ()
        , m_RenderContextPtr()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxPointLightManager::CInternPointLightFacet::~CInternPointLightFacet()
    {
        m_RenderContextPtr = 0;
    }
} // namespace 

namespace 
{
    CGfxPointLightManager::CGfxPointLightManager()
        : m_ShadowShaderVSPtr      ()
        , m_ShadowSMShaderPSPtr    ()
        , m_ShadowRSMShaderPSPtr   ()
        , m_ShadowRSMTexShaderPSPtr()
        , m_LightCameraVSBufferPtr ()
        , m_MainVSBufferPtr        ()
        , m_RSMPSBuffer            ()
        , m_PointLightFacets       ()
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
        m_ShadowShaderVSPtr       = ShaderManager::CompileVS("vs_vm_pnx0.glsl", "main");
        m_ShadowSMShaderPSPtr     = ShaderManager::CompilePS("fs_shadow.glsl", "SM");
        m_ShadowRSMShaderPSPtr    = ShaderManager::CompilePS("fs_shadow.glsl", "RSM_COLOR");
        m_ShadowRSMTexShaderPSPtr = ShaderManager::CompilePS("fs_shadow.glsl", "RSM_TEX");

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
        
        m_MainVSBufferPtr        = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), PerDrawCallConstantBuffer);
        
        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxPointLightManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::OnExit()
    {
        m_ShadowShaderVSPtr       = 0;
        m_ShadowSMShaderPSPtr     = 0;
        m_ShadowRSMShaderPSPtr    = 0;
        m_ShadowRSMTexShaderPSPtr = 0;
        m_LightCameraVSBufferPtr  = 0;
        m_MainVSBufferPtr         = 0;
        m_RSMPSBuffer             = 0;

        m_PointLightFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::Update()
    {
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() == Dt::SLightType::Point)
            {
                Dt::CPointLightFacet*   pDtPointLightFacet = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                CInternPointLightFacet* pGfxPointLightFacet = static_cast<CInternPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                if (pDtPointLightFacet->GetRefreshMode() == Dt::CPointLightFacet::Dynamic)
                {
                    // -----------------------------------------------------------------------------
                    // Update views
                    // -----------------------------------------------------------------------------
                    Gfx::CViewPtr   ShadowViewPtr = pGfxPointLightFacet->m_RenderContextPtr->GetCamera()->GetView();
                    Gfx::CCameraPtr ShadowCameraPtr = pGfxPointLightFacet->m_RenderContextPtr->GetCamera();

                    Base::Float3 LightPosition = rCurrentEntity.GetWorldPosition();
                    Base::Float3 LightDirection = pDtPointLightFacet->GetDirection();

                    // -----------------------------------------------------------------------------
                    // Set view
                    // -----------------------------------------------------------------------------
                    Base::Float3x3 RotationMatrix = Base::Float3x3::s_Identity;

                    RotationMatrix.LookAt(LightPosition, LightPosition + LightDirection, Base::Float3::s_AxisZ);

                    ShadowViewPtr->SetPosition(LightPosition);
                    ShadowViewPtr->SetRotationMatrix(RotationMatrix);

                    // -----------------------------------------------------------------------------
                    // Calculate near and far plane
                    // -----------------------------------------------------------------------------
                    float Near = 0.1f;
                    float Far = pDtPointLightFacet->GetAttenuationRadius() + Near;

                    // -----------------------------------------------------------------------------
                    // Set matrix
                    // -----------------------------------------------------------------------------
                    ShadowCameraPtr->SetFieldOfView(Base::RadiansToDegree(pDtPointLightFacet->GetOuterConeAngle()), 1.0f, Near, Far);

                    ShadowViewPtr->Update();

                    // -----------------------------------------------------------------------------
                    // Render
                    // -----------------------------------------------------------------------------
                    RenderShadows(*pGfxPointLightFacet, pDtPointLightFacet, LightPosition);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        CInternPointLightFacet* pGfxPointLightFacet = 0;

        Dt::CPointLightFacet::EShadowType ShadowType;
        unsigned int ShadowmapSizes[Dt::CPointLightFacet::NumberOfQualities] = { 256, 512, 1024, 2048 };
        unsigned int ShadowmapSize = 0;

        assert(_pEntity != 0);

        // -----------------------------------------------------------------------------
        // Entity check
        // -----------------------------------------------------------------------------
        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;
        if (_pEntity->GetType()     != Dt::SLightType::Point) return;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        Dt::CPointLightFacet* pDtPointLightFacet = static_cast<Dt::CPointLightFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        if (pDtPointLightFacet == nullptr) return;

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            CInternPointLightFacet& rGfxPointLightFacet = AllocatePointLightFacet();

            // -----------------------------------------------------------------------------
            // Set shadow data
            // -----------------------------------------------------------------------------
            ShadowmapSize = ShadowmapSizes[pDtPointLightFacet->GetShadowQuality()];

            ShadowType = pDtPointLightFacet->GetShadowType();

            switch (ShadowType)
            {
            case Dt::CPointLightFacet::HardShadows:        CreateSM(ShadowmapSize, rGfxPointLightFacet); break;
            case Dt::CPointLightFacet::GlobalIllumination: CreateRSM(ShadowmapSize, rGfxPointLightFacet); break;
            }

            // -----------------------------------------------------------------------------
            // Set variables
            // -----------------------------------------------------------------------------
            rGfxPointLightFacet.m_ShadowmapSize = ShadowmapSize;

            rGfxPointLightFacet.m_CurrentShadowType = ShadowType;

            // -----------------------------------------------------------------------------
            // Save facet
            // -----------------------------------------------------------------------------
            _pEntity->SetDetailFacet(Dt::SFacetCategory::Graphic, &rGfxPointLightFacet);

            // -----------------------------------------------------------------------------
            // pGfxPointLightFacet
            // -----------------------------------------------------------------------------
            pGfxPointLightFacet = &rGfxPointLightFacet;
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail) != 0)
        {
            pGfxPointLightFacet = static_cast<CInternPointLightFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

            assert(pGfxPointLightFacet);

            ShadowmapSize = ShadowmapSizes[pDtPointLightFacet->GetShadowQuality()];

            ShadowType = pDtPointLightFacet->GetShadowType();

            if (ShadowmapSize != pGfxPointLightFacet->m_ShadowmapSize || ShadowType != pGfxPointLightFacet->m_CurrentShadowType)
            {
                pGfxPointLightFacet->m_ShadowmapSize = ShadowmapSize;

                pGfxPointLightFacet->m_CurrentShadowType = ShadowType;

                switch (ShadowType)
                {
                case Dt::CPointLightFacet::HardShadows:        CreateSM(ShadowmapSize, *pGfxPointLightFacet); break;
                case Dt::CPointLightFacet::GlobalIllumination: CreateRSM(ShadowmapSize, *pGfxPointLightFacet); break;
                }
            }
        }
        else
        {
            pGfxPointLightFacet = static_cast<CInternPointLightFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));
        }
        
        assert(pGfxPointLightFacet);

        // -----------------------------------------------------------------------------
        // Update views
        // -----------------------------------------------------------------------------
        Gfx::CViewPtr   ShadowViewPtr   = pGfxPointLightFacet->m_RenderContextPtr->GetCamera()->GetView();
        Gfx::CCameraPtr ShadowCameraPtr = pGfxPointLightFacet->m_RenderContextPtr->GetCamera();

        Base::Float3 LightPosition  = _pEntity->GetWorldPosition();
        Base::Float3 LightDirection = pDtPointLightFacet->GetDirection();

        // -----------------------------------------------------------------------------
        // Set view
        // -----------------------------------------------------------------------------
        Base::Float3x3 RotationMatrix = Base::Float3x3::s_Identity;

        RotationMatrix.LookAt(LightPosition, LightPosition + LightDirection, Base::Float3::s_AxisZ);

        ShadowViewPtr->SetPosition(LightPosition);
        ShadowViewPtr->SetRotationMatrix(RotationMatrix);

        // -----------------------------------------------------------------------------
        // Calculate near and far plane
        // -----------------------------------------------------------------------------
        float Near = 0.1f;
        float Far = pDtPointLightFacet->GetAttenuationRadius() + Near;

        // -----------------------------------------------------------------------------
        // Set matrix
        // -----------------------------------------------------------------------------
        ShadowCameraPtr->SetFieldOfView(Base::RadiansToDegree(pDtPointLightFacet->GetOuterConeAngle()), 1.0f, Near, Far);

        ShadowViewPtr->Update();

        // -----------------------------------------------------------------------------
        // Render shadows
        // -----------------------------------------------------------------------------
        RenderShadows(*pGfxPointLightFacet, pDtPointLightFacet, LightPosition);

        // -----------------------------------------------------------------------------
        // Set time
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGfxPointLightFacet->m_TimeStamp = FrameTime;
    }

    // -----------------------------------------------------------------------------

    CGfxPointLightManager::CInternPointLightFacet& CGfxPointLightManager::AllocatePointLightFacet()
    {
        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternPointLightFacet& rGraphicPointLightFacet = m_PointLightFacets.Allocate();
        
        return rGraphicPointLightFacet;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxPointLightManager::CreateRSM(unsigned int _Size, CInternPointLightFacet& _rInternLight)
    {
        unsigned int NumberOfShadowMapPixel = _Size;
        
        // -----------------------------------------------------------------------------
        // Create texture for reflective shadow mapping
        // 0 = Position
        // 1 = Normal
        // 2 = Flux
        // 3 = Depth
        // -----------------------------------------------------------------------------
        CTextureBasePtr ShadowRenderbuffer[4];
        
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = NumberOfShadowMapPixel;
        RendertargetDescriptor.m_NumberOfPixelsV  = NumberOfShadowMapPixel;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        ShadowRenderbuffer[0] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Position
        
        RendertargetDescriptor.m_Binding = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format  = CTextureBase::R16G16B16A16_FLOAT;
        
        ShadowRenderbuffer[1] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Normal
        
        RendertargetDescriptor.m_Binding = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format  = CTextureBase::R16G16B16A16_FLOAT;
        
        ShadowRenderbuffer[2] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Flux
        
        RendertargetDescriptor.m_Binding = CTextureBase::DepthStencilTarget | CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format  = CTextureBase::R32_FLOAT;
        
        ShadowRenderbuffer[3] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth
        
        _rInternLight.m_TextureSMPtr = TextureManager::CreateTextureSet(ShadowRenderbuffer[3]);

        _rInternLight.m_TextureRSMPtr = TextureManager::CreateTextureSet(ShadowRenderbuffer, 4);
        
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
        
        _rInternLight.m_RenderContextPtr = ContextManager::CreateRenderContext();
        
        assert(_rInternLight.m_RenderContextPtr.IsValid());

        _rInternLight.m_RenderContextPtr->SetCamera(CameraPtr);
        _rInternLight.m_RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        _rInternLight.m_RenderContextPtr->SetTargetSet(TargetSetPtr);
        _rInternLight.m_RenderContextPtr->SetRenderState(RenderStatePtr);

        // -----------------------------------------------------------------------------
        // Save camera
        // -----------------------------------------------------------------------------
        _rInternLight.m_CameraPtr = CameraPtr;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxPointLightManager::CreateSM(unsigned int _Size, CInternPointLightFacet& _rInternLight)
    {
        unsigned int NumberOfShadowMapPixel = _Size;
        
        // -----------------------------------------------------------------------------
        // Create texture for shadow mapping
        // -----------------------------------------------------------------------------
        CTextureBasePtr ShadowRenderbuffer[1];
        
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = NumberOfShadowMapPixel;
        RendertargetDescriptor.m_NumberOfPixelsV  = NumberOfShadowMapPixel;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Binding          = CTextureBase::DepthStencilTarget | CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format           = CTextureBase::R32_FLOAT;
        
        ShadowRenderbuffer[0] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth only
        
        _rInternLight.m_TextureSMPtr  = TextureManager::CreateTextureSet(ShadowRenderbuffer, 1);

        _rInternLight.m_TextureRSMPtr = 0;
        
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
        
        _rInternLight.m_RenderContextPtr = ContextManager::CreateRenderContext();
        
        _rInternLight.m_RenderContextPtr->SetCamera(CameraPtr);
        _rInternLight.m_RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        _rInternLight.m_RenderContextPtr->SetTargetSet(TargetSetPtr);
        _rInternLight.m_RenderContextPtr->SetRenderState(RenderStatePtr);

        // -----------------------------------------------------------------------------
        // Save camera
        // -----------------------------------------------------------------------------
        _rInternLight.m_CameraPtr = CameraPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxPointLightManager::RenderShadows(CInternPointLightFacet& _rInternLight, const Dt::CPointLightFacet* _pDtPointLight, const Base::Float3& _rLightPosition)
    {
        if (_rInternLight.m_CurrentShadowType == Dt::CPointLightFacet::NoShadows) return;

        Performance::BeginEvent("Point Light Shadows");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};

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
        SPerLightConstantBuffer* pViewBuffer = static_cast<SPerLightConstantBuffer*>(BufferManager::MapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(0)));
            
        assert(pViewBuffer != nullptr);
            
        pViewBuffer->vs_ViewProjectionMatrix = _rInternLight.m_RenderContextPtr->GetCamera()->GetViewProjectionMatrix();
            
        BufferManager::UnmapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(0));
            
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();
            
        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;
                
            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() != Dt::SActorType::Mesh)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                continue;
            }
                
            // -----------------------------------------------------------------------------
            // Set other graphic data of this entity
            // -----------------------------------------------------------------------------
            CMeshActorFacet* pActorModelFacet = static_cast<CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

            CMeshPtr ModelPtr = pActorModelFacet->GetMesh();
                
            // -----------------------------------------------------------------------------
            // Upload model matrix to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(1)));
                
            assert(pModelBuffer != nullptr);
                
            pModelBuffer->m_ModelMatrix = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();
                
            BufferManager::UnmapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(1));
                
            // -----------------------------------------------------------------------------
            // Render every surface of this entity
            // -----------------------------------------------------------------------------
            unsigned int NumberOfSurfaces = ModelPtr->GetLOD(0)->GetNumberOfSurfaces();
                
            for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++ IndexOfSurface)
            {
                CSurfacePtr SurfacePtr = ModelPtr->GetLOD(0)->GetSurface(IndexOfSurface);
                    
                if (SurfacePtr == nullptr)
                {
                    continue;
                }
                    
                // -----------------------------------------------------------------------------
                // Set material
                // -----------------------------------------------------------------------------
                CMaterialPtr MaterialPtr = SurfacePtr->GetMaterial();

                if (pActorModelFacet->GetMaterial(IndexOfSurface) != 0)
                {
                    MaterialPtr = pActorModelFacet->GetMaterial(IndexOfSurface);
                }

                // -----------------------------------------------------------------------------
                // Set shader + buffer
                // -----------------------------------------------------------------------------
                ContextManager::SetShaderVS(m_ShadowShaderVSPtr);

                ContextManager::SetConstantBufferSetVS(m_LightCameraVSBufferPtr);

                if (_rInternLight.m_CurrentShadowType == Dt::CPointLightFacet::GlobalIllumination)
                {
                    if (MaterialPtr->GetKey().m_HasDiffuseTex)
                    {
                        ContextManager::SetShaderPS(m_ShadowRSMTexShaderPSPtr);

                        ContextManager::SetTextureSetPS(MaterialPtr->GetTextureSetPS());

                        ContextManager::SetSamplerSetPS(MaterialPtr->GetSamplerSetPS());
                    }
                    else
                    {
                        ContextManager::SetShaderPS(m_ShadowRSMShaderPSPtr);
                    }

                    CMaterial::SMaterialAttributes* pMaterialBuffer = static_cast<CMaterial::SMaterialAttributes*>(BufferManager::MapConstantBuffer(m_RSMPSBuffer->GetBuffer(0)));

                    Base::CMemory::Copy(pMaterialBuffer, &MaterialPtr->GetMaterialAttributes(), sizeof(CMaterial::SMaterialAttributes));

                    BufferManager::UnmapConstantBuffer(m_RSMPSBuffer->GetBuffer(0));

                    // -----------------------------------------------------------------------------

                    SPunctualLightProperties* pLightBuffer = static_cast<SPunctualLightProperties*>(BufferManager::MapConstantBuffer(m_RSMPSBuffer->GetBuffer(1)));

                    assert(pLightBuffer != nullptr);

                    float InvSqrAttenuationRadius = _pDtPointLight->GetReciprocalSquaredAttenuationRadius();
                    float AngleScale              = _pDtPointLight->GetAngleScale();
                    float AngleOffset             = _pDtPointLight->GetAngleOffset();

                    pLightBuffer->m_LightPosition  = Base::Float4(_rLightPosition, 1.0f);
                    pLightBuffer->m_LightDirection = Base::Float4(_pDtPointLight->GetDirection(), 0.0f).Normalize();
                    pLightBuffer->m_LightColor     = Base::Float4(_pDtPointLight->GetLightness(), 1.0f);
                    pLightBuffer->m_LightSettings  = Base::Float4(InvSqrAttenuationRadius, AngleScale, AngleOffset, 0.0f);

                    BufferManager::UnmapConstantBuffer(m_RSMPSBuffer->GetBuffer(1));

                    ContextManager::SetConstantBufferSetPS(m_RSMPSBuffer);
                }
                else
                {
                    ContextManager::SetShaderPS(m_ShadowSMShaderPSPtr);
                }
                    
                // -----------------------------------------------------------------------------
                // Get input layout from optimal shader
                // -----------------------------------------------------------------------------
                assert(SurfacePtr->GetKey().m_HasPosition);
                    
                CInputLayoutPtr LayoutPtr = SurfacePtr->GetShaderVS()->GetInputLayout();
                    
                // -----------------------------------------------------------------------------
                // Set items to context manager
                // -----------------------------------------------------------------------------
                ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);
                    
                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);
                    
                ContextManager::SetInputLayout(LayoutPtr);
                    
                ContextManager::SetTopology(STopology::TriangleList);
                    
                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
                    
                ContextManager::ResetTopology();
                    
                ContextManager::ResetInputLayout();
                    
                ContextManager::ResetIndexBuffer();
                    
                ContextManager::ResetVertexBufferSet();
            }
                
            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
        }
            
        ContextManager::ResetConstantBufferSetVS();

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