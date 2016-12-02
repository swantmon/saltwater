
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
#include "data/data_sun_facet.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_facet.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

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
            Base::Float4x4 vs_ViewProjectionMatrix;
        };

        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };

        class CInternSunFacet : public CSunFacet
        {
        public:

            CInternSunFacet();
            ~CInternSunFacet();

        public:

            CRenderContextPtr m_RenderContextPtr;

        private:

            friend class CGfxSunManager;
        };

    private:

        typedef Base::CPool<CInternSunFacet, 2> CSunFacets;

    private:

        CShaderPtr m_ShadowShaderVSPtr;
        CShaderPtr m_ShadowSMShaderPSPtr;
        CSamplerSetPtr m_PSSamplerSetPtr;
        CBufferSetPtr m_LightCameraVSBufferPtr;
        CBufferSetPtr m_MainVSBufferPtr;

        CSunFacets m_SunFacets;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternSunFacet& AllocateSunFacet();

        void CreateSM(unsigned int _Size, CInternSunFacet& _rInternLight);

        void RenderShadows(CInternSunFacet& _rInternLight);
    };
} // namespace

namespace
{
    CGfxSunManager::CInternSunFacet::CInternSunFacet()
        : CSunFacet         ()
        , m_RenderContextPtr()
    {
        
    }

    // -----------------------------------------------------------------------------

    CGfxSunManager::CInternSunFacet::~CInternSunFacet()
    {
        m_RenderContextPtr = 0;
    }
} // namespace 

namespace
{
    CGfxSunManager::CGfxSunManager()
        : m_ShadowShaderVSPtr     ()
        , m_ShadowSMShaderPSPtr   ()
        , m_PSSamplerSetPtr       ()
        , m_LightCameraVSBufferPtr()
        , m_MainVSBufferPtr       ()
        , m_SunFacets             ()
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
        // Sampler
        // -----------------------------------------------------------------------------
        CSamplerPtr Sampler[3];

        Sampler[0] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[1] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[2] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);

        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(Sampler, 3);

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
        
        m_MainVSBufferPtr        = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), PerDrawCallConstantBuffer);
        
        // -----------------------------------------------------------------------------
        // On dirty entities
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxSunManager::OnDirtyEntity));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSunManager::OnExit()
    {
        m_ShadowShaderVSPtr      = 0;
        m_ShadowSMShaderPSPtr    = 0;
        m_PSSamplerSetPtr        = 0;
        m_LightCameraVSBufferPtr = 0;
        m_MainVSBufferPtr        = 0;

        m_SunFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxSunManager::Update()
    {
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() == Dt::SLightType::Sun)
            {
                Dt::CSunLightFacet* pDtSunFacet  = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                CInternSunFacet*    pGfxSunFacet = static_cast<CInternSunFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                if (pDtSunFacet->GetRefreshMode() == Dt::CSunLightFacet::Dynamic)
                {
                    // -----------------------------------------------------------------------------
                    // Update views
                    // -----------------------------------------------------------------------------
                    Gfx::CViewPtr   ShadowViewPtr = pGfxSunFacet->m_RenderContextPtr->GetCamera()->GetView();
                    Gfx::CCameraPtr ShadowCameraPtr = pGfxSunFacet->m_RenderContextPtr->GetCamera();

                    Base::Float3x3 RotationMatrix = Base::Float3x3::s_Identity;

                    Base::Float3 SunPosition = rCurrentEntity.GetWorldPosition();
                    Base::Float3 SunRotation = pDtSunFacet->GetDirection();

                    // -----------------------------------------------------------------------------
                    // Set view
                    // -----------------------------------------------------------------------------
                    RotationMatrix.LookAt(SunPosition, SunPosition + SunRotation, Base::Float3::s_AxisZ);

                    ShadowViewPtr->SetPosition(SunPosition);
                    ShadowViewPtr->SetRotationMatrix(RotationMatrix);

                    // -----------------------------------------------------------------------------
                    // Calculate near and far plane
                    // -----------------------------------------------------------------------------
                    float Radius = 30.0f;

                    float Near = 1.0f;
                    float Far = Radius * 2.0f;

                    // -----------------------------------------------------------------------------
                    // Set matrix
                    // -----------------------------------------------------------------------------
                    ShadowCameraPtr->SetOrthographic(-Radius, Radius, -Radius, Radius, Near, Far);
                    ShadowViewPtr->Update();

                    // -----------------------------------------------------------------------------
                    // Render
                    // -----------------------------------------------------------------------------
                    RenderShadows(*pGfxSunFacet);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSunManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        // -----------------------------------------------------------------------------
        // Entity check
        // -----------------------------------------------------------------------------
        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;
        if (_pEntity->GetType()     != Dt::SLightType::Sun) return;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        Dt::CSunLightFacet* pDtSunFacet = static_cast<Dt::CSunLightFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        if (pDtSunFacet == nullptr) return;

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
            CInternSunFacet& rGfxSunFacet = AllocateSunFacet();

            // -----------------------------------------------------------------------------
            // Set shadow data
            // -----------------------------------------------------------------------------
            CreateSM(2048, rGfxSunFacet);

            // -----------------------------------------------------------------------------
            // Set variables
            // -----------------------------------------------------------------------------
            rGfxSunFacet.m_CameraPtr = rGfxSunFacet.m_RenderContextPtr->GetCamera();

            // -----------------------------------------------------------------------------
            // Save facet
            // -----------------------------------------------------------------------------
            _pEntity->SetDetailFacet(Dt::SFacetCategory::Graphic, &rGfxSunFacet);
        }

        CInternSunFacet* pGfxSunFacet;
            
        pGfxSunFacet = static_cast<CInternSunFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

        // -----------------------------------------------------------------------------
        // Update views
        // -----------------------------------------------------------------------------
        Gfx::CViewPtr   ShadowViewPtr   = pGfxSunFacet->m_RenderContextPtr->GetCamera()->GetView();
        Gfx::CCameraPtr ShadowCameraPtr = pGfxSunFacet->m_RenderContextPtr->GetCamera();

        Base::Float3x3 RotationMatrix = Base::Float3x3::s_Identity;

        Base::Float3 SunPosition = _pEntity->GetWorldPosition();
        Base::Float3 SunRotation = pDtSunFacet->GetDirection();

        // -----------------------------------------------------------------------------
        // Set view
        // -----------------------------------------------------------------------------
        RotationMatrix.LookAt(SunPosition, SunPosition + SunRotation, Base::Float3::s_AxisZ);

        ShadowViewPtr->SetPosition(SunPosition);
        ShadowViewPtr->SetRotationMatrix(RotationMatrix);

        // -----------------------------------------------------------------------------
        // Calculate near and far plane
        // -----------------------------------------------------------------------------
        float Radius = 30.0f;

        float Near = 1.0f;
        float Far = Radius * 2.0f;

        // -----------------------------------------------------------------------------
        // Set matrix
        // -----------------------------------------------------------------------------
        ShadowCameraPtr->SetOrthographic(-Radius, Radius, -Radius, Radius, Near, Far);
        ShadowViewPtr->Update();

        // -----------------------------------------------------------------------------
        // Render shadows
        // -----------------------------------------------------------------------------
        RenderShadows(*pGfxSunFacet);

        // -----------------------------------------------------------------------------
        // Set time
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGfxSunFacet->m_TimeStamp = FrameTime;
    }

    // -----------------------------------------------------------------------------

    CGfxSunManager::CInternSunFacet& CGfxSunManager::AllocateSunFacet()
    {
        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternSunFacet& rGraphicSunFacet = m_SunFacets.Allocate();

        return rGraphicSunFacet;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxSunManager::CreateSM(unsigned int _Size, CInternSunFacet& _rInternLight)
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
    }

    // -----------------------------------------------------------------------------

    void CGfxSunManager::RenderShadows(CInternSunFacet& _rInternLight)
    {
        Performance::BeginEvent("Sun Shadows");

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
        // Set shader
        // -----------------------------------------------------------------------------
        ContextManager::SetShaderVS(m_ShadowShaderVSPtr);
            
        ContextManager::SetShaderPS(m_ShadowSMShaderPSPtr);
            
        // -----------------------------------------------------------------------------
        // Set constant buffer
        // -----------------------------------------------------------------------------
        ContextManager::SetConstantBufferSetVS(m_LightCameraVSBufferPtr);
            
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
            CMeshActorFacet* pGraphicModelActorFacet = static_cast<CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

            CMeshPtr ModelPtr = pGraphicModelActorFacet->GetMesh();
                
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
                CMaterialPtr MaterialPtr;

                if (pGraphicModelActorFacet->GetMaterial(IndexOfSurface) != 0)
                {
                    MaterialPtr = pGraphicModelActorFacet->GetMaterial(IndexOfSurface);
                }
                else
                {
                    MaterialPtr = SurfacePtr->GetMaterial();
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