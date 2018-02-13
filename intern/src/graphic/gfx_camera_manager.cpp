
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_component.h"
#include "data/data_component_manager.h"
#include "data/data_camera_component.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_mesh_component.h"
#include "data/data_texture_2d.h"

#include "graphic/gfx_component.h"
#include "graphic/gfx_component_manager.h"
#include "graphic/gfx_camera_component.h"
#include "graphic/gfx_camera_manager.h"
#include "graphic/gfx_texture_manager.h"

using namespace Gfx;

namespace
{
    class CGfxCameraFacetManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxCameraFacetManager)
        
    public:
        
        CGfxCameraFacetManager();
        ~CGfxCameraFacetManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Update();

    private:

        class CInternCameraActorFacet : public CCameraComponent
        {
        private:

            friend class CGfxCameraFacetManager;
        };
        
    private:

        void OnDirtyComponent(Base::ID _TypeID, Dt::IComponent* _pComponent);
    };
} // namespace

namespace
{
    CGfxCameraFacetManager::CGfxCameraFacetManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxCameraFacetManager::~CGfxCameraFacetManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCameraFacetManager::OnStart()
    {
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CGfxCameraFacetManager::OnDirtyComponent));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCameraFacetManager::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::Update()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::OnDirtyComponent(Base::ID _TypeID, Dt::IComponent* _pComponent)
    {
        if (_TypeID != Base::CTypeInfo::GetTypeID<Dt::CCameraComponent>()) return;

        Dt::CCameraComponent* pCameraComponent = static_cast<Dt::CCameraComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pCameraComponent->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CCameraComponent::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            CInternCameraActorFacet& rGraphicCamera = CComponentManager::GetInstance().Allocate<CInternCameraActorFacet>(pCameraComponent->GetID());

            if (pCameraComponent->GetClearFlag() == Dt::CCameraComponent::Texture)
            {
                unsigned int Hash = pCameraComponent->GetTexture()->GetHash();

                CTexturePtr BackgroundTexturePtr = TextureManager::GetTextureByHash(Hash);

                if (BackgroundTexturePtr.IsValid())
                {
                    rGraphicCamera.SetBackgroundTexture2D(BackgroundTexturePtr);

                    rGraphicCamera.SetBackgroundTextureSet(TextureManager::CreateTextureSet(static_cast<CTexturePtr>(BackgroundTexturePtr)));
                }
            }
        }
        else if ((DirtyFlags & Dt::CCameraComponent::DirtyInfo) != 0)
        {
            CInternCameraActorFacet* pGraphicCamera = CComponentManager::GetInstance().GetComponent<CInternCameraActorFacet>(pCameraComponent->GetID());

            // -----------------------------------------------------------------------------
            // Background
            // -----------------------------------------------------------------------------
            if (pCameraComponent->GetClearFlag() == Dt::CCameraComponent::Texture)
            {
                if (pCameraComponent->GetTexture() != nullptr)
                {
                    unsigned int Hash = pCameraComponent->GetTexture()->GetHash();

                    CTexturePtr BackgroundTexturePtr = TextureManager::GetTextureByHash(Hash);

                    if (BackgroundTexturePtr.IsValid())
                    {
                        pGraphicCamera->SetBackgroundTexture2D(BackgroundTexturePtr);

                        pGraphicCamera->SetBackgroundTextureSet(TextureManager::CreateTextureSet(static_cast<CTexturePtr>(BackgroundTexturePtr)));
                    }
                }
            }

            // -----------------------------------------------------------------------------
            // Other data
            // -----------------------------------------------------------------------------
            Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

            pGraphicCamera->SetTimeStamp(FrameTime);
        }
    }
} // namespace

namespace Gfx
{
namespace CameraManager
{
    void OnStart()
    {
        CGfxCameraFacetManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxCameraFacetManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxCameraFacetManager::GetInstance().Update();
    }
} // namespace CameraActorManager
} // namespace Gfx
