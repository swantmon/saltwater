
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_texture_2d.h"

#include "graphic/gfx_actor_facet.h"
#include "graphic/gfx_actor_manager.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_texture_manager.h"

using namespace Gfx;

namespace
{
    class CGfxActorManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxActorManager)
        
    public:
        
        CGfxActorManager();
        ~CGfxActorManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        void Update();

    private:

        class CInternMeshActorFacet : public CMeshActorFacet
        {
        private:

            friend class CGfxActorManager;
        };

        class CInternARActorFacet : public CARActorFacet
        {
        private:

            friend class CGfxActorManager;
        };

        class CInternCameraActorFacet : public CCameraActorFacet
        {
        private:

            friend class CGfxActorManager;
        };

    private:

        typedef Base::CPool<CInternMeshActorFacet  , 1024> CActorMeshes;
        typedef Base::CPool<CInternARActorFacet    , 32  > CActorARs;
        typedef Base::CPool<CInternCameraActorFacet, 8   > CActorCameras;

        typedef std::vector<Dt::CEntity*> CEntityVector;

    private:

        CEntityVector m_DirtyEntities;
        CActorMeshes  m_ActorMeshes;
        CActorARs     m_ActorARs;
        CActorCameras m_ActorCameras;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdateActor(Dt::CEntity& _rEntity);
        void UpdateActorMesh(Dt::CEntity& _rEntity);
        void UpdateActorAR(Dt::CEntity& _rEntity);
        void UpdateActorCamera(Dt::CEntity& _rEntity);

        void CreateActorMesh(Dt::CEntity& _rEntity);
        void CreateActorAR(Dt::CEntity& _rEntity);
        void CreateActorCamera(Dt::CEntity& _rEntity);
    };
} // namespace

namespace
{
    CGfxActorManager::CGfxActorManager()
        : m_DirtyEntities()
        , m_ActorMeshes  ()
        , m_ActorARs     ()
        , m_ActorCameras ()
    {
        m_DirtyEntities.reserve(65536);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxActorManager::~CGfxActorManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxActorManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxActorManager::OnDirtyEntity));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxActorManager::OnExit()
    {
        Clear();
    }

    void CGfxActorManager::Clear()
    {
        m_ActorMeshes .Clear();
        m_ActorARs    .Clear();
        m_ActorCameras.Clear();

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::Update()
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

    void CGfxActorManager::OnDirtyEntity(Dt::CEntity* _pEntity)
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
            case Dt::SActorType::Mesh:   CreateActorMesh(*_pEntity); break;
            case Dt::SActorType::AR:     CreateActorAR(*_pEntity); break;
            case Dt::SActorType::Camera: CreateActorCamera(*_pEntity); break;
            }
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail) != 0)
        {
            m_DirtyEntities.push_back(_pEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActor(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SActorType::Mesh:   UpdateActorMesh(_rEntity); break;
        case Dt::SActorType::AR:     UpdateActorAR(_rEntity); break;
        case Dt::SActorType::Camera: UpdateActorCamera(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActorMesh(Dt::CEntity& _rEntity)
    {
        Dt::CMeshActorFacet* pDataActorModelFacet;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        pDataActorModelFacet = static_cast<Dt::CMeshActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataActorModelFacet);

        // -----------------------------------------------------------------------------
        // Get facet
        // -----------------------------------------------------------------------------
        CInternMeshActorFacet* pGraphicActorModelFacet = static_cast<CInternMeshActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

        // -----------------------------------------------------------------------------
        // Model
        // -----------------------------------------------------------------------------
        CMeshPtr ModelPtr = pGraphicActorModelFacet->GetMesh();

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDesc;

        CLODPtr ModelLODPtr = ModelPtr->GetLOD(0);

        for (unsigned int NumberOfSurface = 0; NumberOfSurface < ModelLODPtr->GetNumberOfSurfaces(); ++NumberOfSurface)
        {
            CSurfacePtr SurfacePtr = ModelLODPtr->GetSurface(NumberOfSurface);

            Dt::CMaterial* pDataMaterial = pDataActorModelFacet->GetMaterial(NumberOfSurface);
            CMaterialPtr MaterialPtr = pGraphicActorModelFacet->GetMaterial(NumberOfSurface);

            if (pDataMaterial == nullptr)
            {
                pDataMaterial = pDataActorModelFacet->GetMesh()->GetLOD(0)->GetSurface(NumberOfSurface)->GetMaterial();
            }

            if (MaterialPtr == nullptr)
            {
                MaterialPtr = SurfacePtr->GetMaterial();
            }

            if (pDataMaterial == nullptr || MaterialPtr == nullptr)
            {
                continue;
            }

            // -----------------------------------------------------------------------------
            // Get key of surface
            // -----------------------------------------------------------------------------
            CSurface::SSurfaceKey SurfaceKey = SurfacePtr->GetKey();

            // -----------------------------------------------------------------------------
            // Material description
            // -----------------------------------------------------------------------------
            MaterialDesc.m_ID        = SurfaceKey.m_Key;
            MaterialDesc.m_pMaterial = pDataMaterial;

            // -----------------------------------------------------------------------------
            // Update material
            // -----------------------------------------------------------------------------
            MaterialManager::UpdateMaterial(MaterialPtr, MaterialDesc);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActorAR(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActorCamera(Dt::CEntity& _rEntity)
    {
        STextureDescriptor       TextureDescriptor;
        Dt::CCameraActorFacet*   pDataCamera;
        Gfx::CCameraActorFacet*  pGraphicCamera;

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

    void CGfxActorManager::CreateActorMesh(Dt::CEntity& _rEntity)
    {
        Dt::CMeshActorFacet* pDataActorModelFacet;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        pDataActorModelFacet = static_cast<Dt::CMeshActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataActorModelFacet);

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternMeshActorFacet& rGraphicActorModelFacet = m_ActorMeshes.Allocate();

        // -----------------------------------------------------------------------------
        // Prepare storage data : Model
        // -----------------------------------------------------------------------------
        SMeshDescriptor ModelDesc;

        ModelDesc.m_pModel = pDataActorModelFacet->GetMesh();

        CMeshPtr NewModelPtr = MeshManager::CreateMesh(ModelDesc);

        rGraphicActorModelFacet.SetMesh(NewModelPtr);

        // -----------------------------------------------------------------------------
        // Prepare storage data : Material
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDesc;

        CLODPtr ModelLODPtr = NewModelPtr->GetLOD(0);

        for (unsigned int NumberOfSurface = 0; NumberOfSurface < ModelLODPtr->GetNumberOfSurfaces(); ++NumberOfSurface)
        {
            Dt::CMaterial* pDataMaterial = pDataActorModelFacet->GetMaterial(NumberOfSurface);

            if (pDataMaterial != 0)
            {
                // -----------------------------------------------------------------------------
                // Get key of surface
                // -----------------------------------------------------------------------------
                CSurface::SSurfaceKey SurfaceKey = ModelLODPtr->GetSurface(NumberOfSurface)->GetKey();

                // -----------------------------------------------------------------------------
                // Material description
                // -----------------------------------------------------------------------------
                MaterialDesc.m_ID        = SurfaceKey.m_Key;
                MaterialDesc.m_pMaterial = pDataMaterial;

                // -----------------------------------------------------------------------------
                // Create and set material
                // -----------------------------------------------------------------------------
                CMaterialPtr NewMaterialPtr = MaterialManager::CreateMaterial(MaterialDesc);

                rGraphicActorModelFacet.SetMaterial(NumberOfSurface, NewMaterialPtr);
            }
        }        

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicActorModelFacet);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::CreateActorAR(Dt::CEntity& _rEntity)
    {
        Dt::CMeshActorFacet* pDataActorModelFacet;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        pDataActorModelFacet = static_cast<Dt::CMeshActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataActorModelFacet);

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternARActorFacet& rGraphicActorARFacet = m_ActorARs.Allocate();

        // -----------------------------------------------------------------------------
        // Prepare storage data : Model
        // -----------------------------------------------------------------------------
        SMeshDescriptor ModelDesc;

        ModelDesc.m_pModel = pDataActorModelFacet->GetMesh();

        CMeshPtr NewModelPtr = MeshManager::CreateMesh(ModelDesc);

        rGraphicActorARFacet.SetMesh(NewModelPtr);

        // -----------------------------------------------------------------------------
        // Prepare storage data : Material
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDesc;

        CLODPtr ModelLODPtr = NewModelPtr->GetLOD(0);

        for (unsigned int NumberOfSurface = 0; NumberOfSurface < ModelLODPtr->GetNumberOfSurfaces(); ++NumberOfSurface)
        {
            Dt::CMaterial* pDataMaterial = pDataActorModelFacet->GetMaterial(NumberOfSurface);

            if (pDataMaterial != 0)
            {
                // -----------------------------------------------------------------------------
                // Get key of surface
                // -----------------------------------------------------------------------------
                CSurface::SSurfaceKey SurfaceKey = ModelLODPtr->GetSurface(NumberOfSurface)->GetKey();

                // -----------------------------------------------------------------------------
                // Material description
                // -----------------------------------------------------------------------------
                MaterialDesc.m_ID = SurfaceKey.m_Key;
                MaterialDesc.m_pMaterial = pDataMaterial;

                // -----------------------------------------------------------------------------
                // Create and set material
                // -----------------------------------------------------------------------------
                CMaterialPtr NewMaterialPtr = MaterialManager::CreateMaterial(MaterialDesc);

                rGraphicActorARFacet.SetMaterial(NumberOfSurface, NewMaterialPtr);
            }
        }

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicActorARFacet);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::CreateActorCamera(Dt::CEntity& _rEntity)
    {
        STextureDescriptor      TextureDescriptor;
        Dt::CCameraActorFacet*  pDataCamera;

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
namespace ActorManager
{
    void OnStart()
    {
        CGfxActorManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxActorManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxActorManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxActorManager::GetInstance().Update();
    }
} // namespace ActorManager
} // namespace Gfx
