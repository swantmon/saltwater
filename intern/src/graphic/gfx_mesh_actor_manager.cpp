
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_mesh_actor_facet.h"
#include "data/data_texture_2d.h"

#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_mesh_actor_manager.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_texture_manager.h"

using namespace Gfx;

namespace
{
    class CGfxMeshFacetManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxMeshFacetManager)
        
    public:
        
        CGfxMeshFacetManager();
        ~CGfxMeshFacetManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        void Update();

    private:

        class CInternMeshActorFacet : public CMeshActorFacet
        {
        private:

            friend class CGfxMeshFacetManager;
        };

    private:

        typedef Base::CPool<CInternMeshActorFacet, 1024> CActorMeshes;

        typedef std::vector<Dt::CEntity*> CEntityVector;

    private:

        CEntityVector m_DirtyEntities;
        CActorMeshes  m_ActorMeshes;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdateActor(Dt::CEntity& _rEntity);
        void UpdateActorMesh(Dt::CEntity& _rEntity);

        void CreateActorMesh(Dt::CEntity& _rEntity);
    };
} // namespace

namespace
{
    CGfxMeshFacetManager::CGfxMeshFacetManager()
        : m_DirtyEntities()
        , m_ActorMeshes  ()
    {
        m_DirtyEntities.reserve(65536);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxMeshFacetManager::~CGfxMeshFacetManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMeshFacetManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxMeshFacetManager::OnDirtyEntity));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMeshFacetManager::OnExit()
    {
        Clear();
    }

    void CGfxMeshFacetManager::Clear()
    {
        m_ActorMeshes .Clear();

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshFacetManager::Update()
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

    void CGfxMeshFacetManager::OnDirtyEntity(Dt::CEntity* _pEntity)
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
            }
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail) != 0)
        {
            m_DirtyEntities.push_back(_pEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshFacetManager::UpdateActor(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SActorType::Mesh:   UpdateActorMesh(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshFacetManager::UpdateActorMesh(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshFacetManager::CreateActorMesh(Dt::CEntity& _rEntity)
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
                // Create and set material
                // -----------------------------------------------------------------------------
                unsigned int Hash = pDataMaterial->GetHash();

                CMaterialPtr NewMaterialPtr = MaterialManager::GetMaterialByHash(Hash);

                rGraphicActorModelFacet.SetMaterial(NumberOfSurface, NewMaterialPtr);
            }
        }        

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicActorModelFacet);
    }
} // namespace

namespace Gfx
{
namespace MeshActorManager
{
    void OnStart()
    {
        CGfxMeshFacetManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxMeshFacetManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxMeshFacetManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxMeshFacetManager::GetInstance().Update();
    }
} // namespace MeshActorManager
} // namespace Gfx
