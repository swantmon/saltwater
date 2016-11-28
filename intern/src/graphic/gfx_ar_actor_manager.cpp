
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_ar_actor_facet.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_mesh_actor_facet.h"
#include "data/data_texture_2d.h"

#include "graphic/gfx_ar_actor_facet.h"
#include "graphic/gfx_ar_actor_manager.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_texture_manager.h"

using namespace Gfx;

namespace
{
    class CGfxARMeshManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxARMeshManager)
        
    public:
        
        CGfxARMeshManager();
        ~CGfxARMeshManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        void Update();

    private:

        class CInternARActorFacet : public CARActorFacet
        {
        private:

            friend class CGfxARMeshManager;
        };

    private:

        typedef Base::CPool<CInternARActorFacet, 32  > CActorARs;

        typedef std::vector<Dt::CEntity*> CEntityVector;

    private:

        CEntityVector m_DirtyEntities;
        CActorARs     m_ActorARs;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdateActor(Dt::CEntity& _rEntity);
        void UpdateActorAR(Dt::CEntity& _rEntity);

        void CreateActorAR(Dt::CEntity& _rEntity);
    };
} // namespace

namespace
{
    CGfxARMeshManager::CGfxARMeshManager()
        : m_DirtyEntities()
        , m_ActorARs     ()
    {
        m_DirtyEntities.reserve(65536);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxARMeshManager::~CGfxARMeshManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxARMeshManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxARMeshManager::OnDirtyEntity));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxARMeshManager::OnExit()
    {
        Clear();
    }

    void CGfxARMeshManager::Clear()
    {
        m_ActorARs.Clear();

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxARMeshManager::Update()
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

    void CGfxARMeshManager::OnDirtyEntity(Dt::CEntity* _pEntity)
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
            case Dt::SActorType::AR:     CreateActorAR(*_pEntity); break;
            }
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail) != 0)
        {
            m_DirtyEntities.push_back(_pEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxARMeshManager::UpdateActor(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SActorType::AR:     UpdateActorAR(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxARMeshManager::UpdateActorAR(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxARMeshManager::CreateActorAR(Dt::CEntity& _rEntity)
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
                // Create and set material
                // -----------------------------------------------------------------------------
                unsigned int Hash = pDataMaterial->GetHash();

                CMaterialPtr NewMaterialPtr = MaterialManager::GetMaterialByHash(Hash);

                rGraphicActorARFacet.SetMaterial(NumberOfSurface, NewMaterialPtr);
            }
            else
            {
                CMaterialPtr NewMaterialPtr = MaterialManager::GetDefaultMaterial();

                rGraphicActorARFacet.SetMaterial(NumberOfSurface, NewMaterialPtr);
            }
        }

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicActorARFacet);
    }
} // namespace

namespace Gfx
{
namespace ARActorManager
{
    void OnStart()
    {
        CGfxARMeshManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxARMeshManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxARMeshManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxARMeshManager::GetInstance().Update();
    }
} // namespace ARActorManager
} // namespace Gfx
