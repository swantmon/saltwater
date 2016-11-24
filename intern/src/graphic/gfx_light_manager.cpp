
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_cube.h"

#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_light_facet.h"
#include "graphic/gfx_light_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxLightManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLightManager)
        
    public:
        
        CGfxLightManager();
        ~CGfxLightManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        void Update();

    private:

        class CInternPointLightFacet : public CPointLightFacet
        {
        private:

            friend class CGfxLightManager;
        };

        class CInternSunLightFacet : public CSunLightFacet
        {
        private:

            friend class CGfxLightManager;
        };

    private:

        typedef Base::CPool<CInternPointLightFacet, 64>       CPointLights;
        typedef Base::CPool<CInternSunLightFacet, 64>         CSunLights;
        
        typedef std::vector<Dt::CEntity*>  CEntityVector;

    private:

        CEntityVector m_DirtyEntities;
        CPointLights  m_PointLights;
        CSunLights    m_SunLights;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdateLight(Dt::CEntity& _rEntity);
        void UpdatePointLight(Dt::CEntity& _rEntity);
        void UpdateSunLight(Dt::CEntity& _rEntity);
        void UpdateGlobalProbeLight(Dt::CEntity& _rEntity);

        void CreatePointLight(Dt::CEntity& _rEntity);
        void CreateSunLight(Dt::CEntity& _rEntity);
        void CreateGlobalProbeLight(Dt::CEntity& _rEntity);

        template<class TShadowLight>
        void CreateRSM(unsigned int _Size, TShadowLight& _rInternLight);

        template<class TShadowLight>
        void CreateShadowmap(unsigned int _Size, TShadowLight& _rInternLight);
    };
} // namespace

namespace
{
    CGfxLightManager::CGfxLightManager()
        : m_PointLights      ()
        , m_SunLights        ()
    {
        m_DirtyEntities.reserve(65536);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxLightManager::~CGfxLightManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxLightManager::OnDirtyEntity));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightManager::OnExit()
    {
        Clear();
    }

    void CGfxLightManager::Clear()
    {
        m_PointLights.Clear();
        m_SunLights  .Clear();

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::Update()
    {
        CEntityVector::iterator CurrentDirtyEntity = m_DirtyEntities.begin();
        CEntityVector::iterator EndOfDirtyEntities = m_DirtyEntities.end();

        for (; CurrentDirtyEntity != EndOfDirtyEntities; ++CurrentDirtyEntity)
        {
            UpdateLight(**CurrentDirtyEntity);
        }

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create a light
            // -----------------------------------------------------------------------------
            switch (_pEntity->GetType())
            {
            case Dt::SLightType::Point: CreatePointLight(*_pEntity); break;
            case Dt::SLightType::Sun:   CreateSunLight(*_pEntity); break;
            }
        }

        m_DirtyEntities.push_back(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::UpdateLight(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SLightType::Point: UpdatePointLight(_rEntity); break;
        case Dt::SLightType::Sun:   UpdateSunLight(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::UpdatePointLight(Dt::CEntity& _rEntity)
    {
        Dt::CPointLightFacet*   pDataPointLightFacet;
        Gfx::CPointLightFacet*  pGraphicPointLightFacet;
        CInternPointLightFacet* pInternPointLightFacet;

        pDataPointLightFacet    = static_cast<Dt::CPointLightFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));
        pGraphicPointLightFacet = static_cast<Gfx::CPointLightFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

        pInternPointLightFacet = static_cast<CInternPointLightFacet*>(pGraphicPointLightFacet);

        // -----------------------------------------------------------------------------
        // Update views
        // -----------------------------------------------------------------------------
        Gfx::CViewPtr   ShadowViewPtr   = pInternPointLightFacet->m_RenderContextPtr->GetCamera()->GetView();
        Gfx::CCameraPtr ShadowCameraPtr = pInternPointLightFacet->m_RenderContextPtr->GetCamera();

        Base::Float3 LightPosition  = _rEntity.GetWorldPosition();
        Base::Float3 LightDirection = pDataPointLightFacet->GetDirection();

        // -----------------------------------------------------------------------------
        // Set view
        // -----------------------------------------------------------------------------
        Base::Float3x3 RotationMatrix = Base::Float3x3::s_Identity;

        RotationMatrix.LookAt(LightPosition, LightPosition - LightDirection, Base::Float3(0.0f, 1.0f, 0.0f));

        ShadowViewPtr->SetPosition(LightPosition);
        ShadowViewPtr->SetRotationMatrix(RotationMatrix);

        // -----------------------------------------------------------------------------
        // Calculate near and far plane
        // -----------------------------------------------------------------------------
        float Near = 0.1f;
        float Far = pDataPointLightFacet->GetAttenuationRadius() + Near;

        // -----------------------------------------------------------------------------
        // Set matrix
        // -----------------------------------------------------------------------------
        ShadowCameraPtr->SetFieldOfView(Base::RadiansToDegree(pDataPointLightFacet->GetOuterConeAngle()), 1.0f, Near, Far);

        ShadowViewPtr->Update();

        // -----------------------------------------------------------------------------
        // Other data
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGraphicPointLightFacet->SetTimeStamp(FrameTime);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::UpdateSunLight(Dt::CEntity& _rEntity)
    {
        Dt::CSunLightFacet*   pDataSunLightFacet;
        Gfx::CSunLightFacet*  pGraphicSunLightFacet;
        CInternSunLightFacet* pInternSunLightFacet;

        pDataSunLightFacet    = static_cast<Dt::CSunLightFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));
        pGraphicSunLightFacet = static_cast<Gfx::CSunLightFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

        pInternSunLightFacet = static_cast<CInternSunLightFacet*>(pGraphicSunLightFacet);

        // -----------------------------------------------------------------------------
        // Update views
        // -----------------------------------------------------------------------------
        Gfx::CViewPtr   ShadowViewPtr   = pInternSunLightFacet->m_RenderContextPtr->GetCamera()->GetView();
        Gfx::CCameraPtr ShadowCameraPtr = pInternSunLightFacet->m_RenderContextPtr->GetCamera();

        Base::Float3x3 RotationMatrix = Base::Float3x3::s_Identity;

        Base::Float3 SunPosition = _rEntity.GetWorldPosition();
        Base::Float3 SunRotation = pDataSunLightFacet->GetDirection();

        // -----------------------------------------------------------------------------
        // Set view
        // -----------------------------------------------------------------------------
        RotationMatrix.LookAt(SunPosition, SunPosition - SunRotation, Base::Float3(0.0f, 1.0f, 0.0f));

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
        // Other data
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGraphicSunLightFacet->SetTimeStamp(FrameTime);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::CreatePointLight(Dt::CEntity& _rEntity)
    {
        unsigned int ShadowmapSizes[Dt::CPointLightFacet::NumberOfQualities] = { 256, 512, 1024, 2048 };
        unsigned int ShadowmapSize;

        Dt::CPointLightFacet* pDataPointLightFacet;

        // -----------------------------------------------------------------------------
        // Get data point light
        // -----------------------------------------------------------------------------
        pDataPointLightFacet = static_cast<Dt::CPointLightFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataPointLightFacet);

        // -----------------------------------------------------------------------------
        // Create graphic point light
        // -----------------------------------------------------------------------------
        CInternPointLightFacet& rGraphicPointLightFacet = m_PointLights.Allocate();

        // -----------------------------------------------------------------------------
        // Set shadow data
        // -----------------------------------------------------------------------------
        ShadowmapSize = ShadowmapSizes[pDataPointLightFacet->GetShadowQuality()];

        switch (pDataPointLightFacet->GetShadowType())
        {
        case Dt::CPointLightFacet::HardShadows:        CreateShadowmap(ShadowmapSize, rGraphicPointLightFacet); break;
        case Dt::CPointLightFacet::GlobalIllumination: CreateRSM(ShadowmapSize, rGraphicPointLightFacet); break;
        }

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicPointLightFacet);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightManager::CreateSunLight(Dt::CEntity& _rEntity)
    {
        Dt::CSunLightFacet* pDataSunLightFacet;

        // -----------------------------------------------------------------------------
        // Get sun data informations
        // -----------------------------------------------------------------------------
        pDataSunLightFacet = static_cast<Dt::CSunLightFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataSunLightFacet);

        // -----------------------------------------------------------------------------
        // Create graphic sun light
        // -----------------------------------------------------------------------------
        CInternSunLightFacet& rGraphicSunLightFacet = m_SunLights.Allocate();

        // -----------------------------------------------------------------------------
        // Set shadow data
        // -----------------------------------------------------------------------------
        CreateShadowmap(2048, rGraphicSunLightFacet);

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicSunLightFacet);
    }

    // -----------------------------------------------------------------------------
    
    template<class TShadowLight>
    void CGfxLightManager::CreateRSM(unsigned int _Size, TShadowLight& _rInternLight)
    {
        unsigned int NumberOfShadowMapPixel = _Size;
        
        // -----------------------------------------------------------------------------
        // Create texture for shadow mapping
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
        RendertargetDescriptor.m_Format  = CTextureBase::R8G8B8A8_UBYTE;
        
        ShadowRenderbuffer[2] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Flux
        
        RendertargetDescriptor.m_Binding = CTextureBase::DepthStencilTarget | CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format  = CTextureBase::R32_FLOAT;
        
        ShadowRenderbuffer[3] = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth only
        
        _rInternLight.m_TextureSMPtr = TextureManager::CreateTextureSet(ShadowRenderbuffer, 4);
        
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
        
        _rInternLight.m_RenderContextPtr->SetCamera(CameraPtr);
        _rInternLight.m_RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        _rInternLight.m_RenderContextPtr->SetTargetSet(TargetSetPtr);
        _rInternLight.m_RenderContextPtr->SetRenderState(RenderStatePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class TShadowLight>
    void CGfxLightManager::CreateShadowmap(unsigned int _Size, TShadowLight& _rInternLight)
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
} // namespace

namespace Gfx
{
namespace LightManager
{
    void OnStart()
    {
        CGfxLightManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxLightManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxLightManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxLightManager::GetInstance().Update();
    }
} // namespace LightManager
} // namespace Gfx