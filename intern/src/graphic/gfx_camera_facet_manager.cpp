
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_ar_mesh_facet.h"
#include "data/data_camera_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_mesh_facet.h"
#include "data/data_texture_2d.h"

#include "graphic/gfx_camera_facet.h"
#include "graphic/gfx_camera_facet_manager.h"
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

        void Clear();

        void Update();

    private:

        class CInternCameraActorFacet : public CCameraActorFacet
        {
        private:

            friend class CGfxCameraFacetManager;
        };

    private:
        typedef Base::CPool<CInternCameraActorFacet, 8> CActorCameras;

        typedef std::vector<Dt::CEntity*> CEntityVector;

    private:

        CEntityVector m_DirtyEntities;
        CActorCameras m_ActorCameras;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdateActor(Dt::CEntity& _rEntity);
        void UpdateActorCamera(Dt::CEntity& _rEntity);

        void CreateActorCamera(Dt::CEntity& _rEntity);
    };
} // namespace

namespace
{
    CGfxCameraFacetManager::CGfxCameraFacetManager()
        : m_DirtyEntities()
        , m_ActorCameras ()
    {
        m_DirtyEntities.reserve(65536);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxCameraFacetManager::~CGfxCameraFacetManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCameraFacetManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxCameraFacetManager::OnDirtyEntity));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCameraFacetManager::OnExit()
    {
        Clear();
    }

    void CGfxCameraFacetManager::Clear()
    {
        m_ActorCameras.Clear();

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::Update()
    {
        CEntityVector::iterator CurrentDirtyEntity = m_DirtyEntities.begin();
        CEntityVector::iterator EndOfDirtyEntities = m_DirtyEntities.end();

        for (; CurrentDirtyEntity != EndOfDirtyEntities; ++CurrentDirtyEntity)
        {
            UpdateActor(**CurrentDirtyEntity);
        }

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (_pEntity->GetCategory() != Dt::SEntityCategory::Actor) return;

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create a actor
            // -----------------------------------------------------------------------------
            switch (_pEntity->GetType())
            {
            case Dt::SActorType::Camera: CreateActorCamera(*_pEntity); break;
            }
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail) != 0)
        {
            m_DirtyEntities.push_back(_pEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::UpdateActor(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SActorType::Camera: UpdateActorCamera(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::UpdateActorCamera(Dt::CEntity& _rEntity)
    {
        Dt::CCameraActorFacet*  pDataCamera;
        Gfx::CCameraActorFacet* pGraphicCamera;

        pDataCamera    = static_cast<Dt::CCameraActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));
        pGraphicCamera = static_cast<Gfx::CCameraActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

        // -----------------------------------------------------------------------------
        // Background
        // -----------------------------------------------------------------------------
        if (pDataCamera->GetClearFlag() == Dt::CCameraActorFacet::Texture)
        {
            if (pDataCamera->GetTexture() != nullptr)
            {
                unsigned int Hash = pDataCamera->GetTexture()->GetHash();

                CTexture2DPtr BackgroundTexturePtr = TextureManager::GetTexture2DByHash(Hash);

                if (BackgroundTexturePtr.IsValid())
                {
                    pGraphicCamera->SetBackgroundTexture2D(BackgroundTexturePtr);

                    pGraphicCamera->SetBackgroundTextureSet(TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(BackgroundTexturePtr)));
                }
            }
        }        

        // -----------------------------------------------------------------------------
        // Other data
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pGraphicCamera->SetTimeStamp(FrameTime);
    }

    // -----------------------------------------------------------------------------

    void CGfxCameraFacetManager::CreateActorCamera(Dt::CEntity& _rEntity)
    {
        Dt::CCameraActorFacet* pDataCamera;

        // -----------------------------------------------------------------------------
        // Get sun data informations
        // -----------------------------------------------------------------------------
        pDataCamera = static_cast<Dt::CCameraActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataCamera);

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternCameraActorFacet& rGraphicCamera = m_ActorCameras.Allocate();

        if (pDataCamera->GetClearFlag() == Dt::CCameraActorFacet::Texture)
        {
            unsigned int Hash = pDataCamera->GetTexture()->GetHash();

            CTexture2DPtr BackgroundTexturePtr = TextureManager::GetTexture2DByHash(Hash);

            if (BackgroundTexturePtr.IsValid())
            {
                rGraphicCamera.SetBackgroundTexture2D(BackgroundTexturePtr);

                rGraphicCamera.SetBackgroundTextureSet(TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(BackgroundTexturePtr)));
            }
        }

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicCamera);
    }
} // namespace

namespace Gfx
{
namespace CameraFacetManager
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

    void Clear()
    {
        CGfxCameraFacetManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxCameraFacetManager::GetInstance().Update();
    }
} // namespace CameraFacetManager
} // namespace Gfx
