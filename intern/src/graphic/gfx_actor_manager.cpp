
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_sphere.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"

#include "graphic/gfx_actor_facet.h"
#include "graphic/gfx_actor_manager.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_model_manager.h"

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

        class CInternModelActorFacet : public CModelActorFacet
        {
        private:

            friend class CGfxActorManager;
        };

        class CInternARActorFacet : public CARActorFacet
        {
        private:

            friend class CGfxActorManager;
        };

    private:

        typedef Base::CPool<CInternModelActorFacet, 1024> CActorModels;
        typedef Base::CPool<CInternARActorFacet, 32> CActorARs;

        typedef std::vector<Dt::CEntity*> CEntityVector;

    private:

        CEntityVector m_DirtyEntities;
        CActorModels  m_ActorModels;
        CActorARs     m_ActorARs;
        
    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdateActor(Dt::CEntity& _rEntity);
        void UpdateActorModel(Dt::CEntity& _rEntity);
        void UpdateActorAR(Dt::CEntity& _rEntity);

        void CreateActorModel(Dt::CEntity& _rEntity);
        void CreateActorAR(Dt::CEntity& _rEntity);
    };
} // namespace

namespace
{
    CGfxActorManager::CGfxActorManager()
        : m_DirtyEntities()
        , m_ActorModels  ()
        , m_ActorARs     ()
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
        m_ActorModels.Clear();
        m_ActorARs   .Clear();

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
            // Create a light
            // -----------------------------------------------------------------------------
            switch (_pEntity->GetType())
            {
            case Dt::SActorType::Model: CreateActorModel(*_pEntity); break;
            case Dt::SActorType::AR:    CreateActorAR(*_pEntity); break;
            }
        }

        m_DirtyEntities.push_back(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActor(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SActorType::Model: UpdateActorModel(_rEntity); break;
        case Dt::SActorType::AR:    UpdateActorAR(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActorModel(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::UpdateActorAR(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::CreateActorModel(Dt::CEntity& _rEntity)
    {
        Dt::CModelActorFacet* pDataActorModelFacet;

        // -----------------------------------------------------------------------------
        // Get data point light
        // -----------------------------------------------------------------------------
        pDataActorModelFacet = static_cast<Dt::CModelActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataActorModelFacet);

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternModelActorFacet& rGraphicActorModelFacet = m_ActorModels.Allocate();

        // -----------------------------------------------------------------------------
        // Prepare storage data : Model
        // -----------------------------------------------------------------------------
        SModelDescriptor ModelDesc;

        ModelDesc.m_pModel = pDataActorModelFacet->GetModel();

        CModelPtr NewModelPtr = ModelManager::CreateModel(ModelDesc);

        rGraphicActorModelFacet.SetModel(NewModelPtr);

        // -----------------------------------------------------------------------------
        // Prepare storage data : Material
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDesc;

        CLODPtr ModelLODPtr = NewModelPtr->GetLOD(0);

        Dt::CMaterial* pDataMaterial = pDataActorModelFacet->GetMaterial();

        if (pDataMaterial != 0)
        {
            // -----------------------------------------------------------------------------
            // Get key of surface
            // -----------------------------------------------------------------------------
            CSurface::SSurfaceKey SurfaceKey = ModelLODPtr->GetSurface(0)->GetKey();

            // -----------------------------------------------------------------------------
            // Material description
            // -----------------------------------------------------------------------------
            MaterialDesc.m_ID = SurfaceKey.m_Key;
            MaterialDesc.m_pMaterial = pDataMaterial;

            // -----------------------------------------------------------------------------
            // Create and set material
            // -----------------------------------------------------------------------------
            CMaterialPtr NewMaterialPtr = MaterialManager::CreateMaterial(MaterialDesc);

            rGraphicActorModelFacet.SetMaterial(NewMaterialPtr);
        }

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicActorModelFacet);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorManager::CreateActorAR(Dt::CEntity& _rEntity)
    {
        Dt::CModelActorFacet* pDataActorModelFacet;

        // -----------------------------------------------------------------------------
        // Get data point light
        // -----------------------------------------------------------------------------
        pDataActorModelFacet = static_cast<Dt::CModelActorFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pDataActorModelFacet);

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternARActorFacet& rGraphicActorARFacet = m_ActorARs.Allocate();

        // -----------------------------------------------------------------------------
        // Prepare storage data : Model
        // -----------------------------------------------------------------------------
        SModelDescriptor ModelDesc;

        ModelDesc.m_pModel = pDataActorModelFacet->GetModel();

        CModelPtr NewModelPtr = ModelManager::CreateModel(ModelDesc);

        rGraphicActorARFacet.SetModel(NewModelPtr);

        // -----------------------------------------------------------------------------
        // Prepare storage data : Material
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDesc;

        CLODPtr ModelLODPtr = NewModelPtr->GetLOD(0);

        Dt::CMaterial* pDataMaterial = pDataActorModelFacet->GetMaterial();

        if (pDataMaterial != 0)
        {
            // -----------------------------------------------------------------------------
            // Get key of surface
            // -----------------------------------------------------------------------------
            CSurface::SSurfaceKey SurfaceKey = ModelLODPtr->GetSurface(0)->GetKey();

            // -----------------------------------------------------------------------------
            // Material description
            // -----------------------------------------------------------------------------
            MaterialDesc.m_ID = SurfaceKey.m_Key;
            MaterialDesc.m_pMaterial = pDataMaterial;

            // -----------------------------------------------------------------------------
            // Create and set material
            // -----------------------------------------------------------------------------
            CMaterialPtr NewMaterialPtr = MaterialManager::CreateMaterial(MaterialDesc);

            rGraphicActorARFacet.SetMaterial(NewMaterialPtr);
        }

        // -----------------------------------------------------------------------------
        // Save facet
        // -----------------------------------------------------------------------------
        _rEntity.SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicActorARFacet);
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
