
#include "data/data_precompiled.h"

#include "base/base_exception.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"
#include "data/data_transformation_facet.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::EntityManager;

namespace
{
	std::string g_PathToAssets = "../assets/";
} // namespace 

namespace
{
    class CDtLvlEntityManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtLvlEntityManager);
        
    public:
        
        CDtLvlEntityManager();
        ~CDtLvlEntityManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CEntity& CreateEntities(const SSceneDescriptor& _rDescriptor);

        CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID = CEntity::s_InvalidID);

        void FreeEntity(CEntity& _rEntity);

        void MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags);

        void Update();

        void RegisterDirtyEntityHandler(CEntityDelegate _NewDelegate);
        
    private:
        
        class CInternEntity : public CEntity
        {
        private:
            friend class CDtLvlEntityManager;
        };

        class CInternHierarchyFacet : public CHierarchyFacet
        {
        private:
            friend class CDtLvlEntityManager;
        };

        class CInternTransformationFacet : public CTransformationFacet
        {
        private:
            friend class CDtLvlEntityManager;
        };
        
    private:
        
        typedef Base::CPool<CInternEntity, 2048>              CEntityPool;
        typedef Base::CPool<CInternHierarchyFacet, 2048>      CHierarchyFacetPool;
        typedef Base::CPool<CInternTransformationFacet, 2048> CTransformationFacetPool;
        typedef std::vector<CEntity*>                         CEntityVector;
        typedef std::vector<CEntityDelegate>                  CEntityDelegates;

    private:
        
        CEntityPool              m_Entities;
        CHierarchyFacetPool      m_HierarchyFacets;
        CTransformationFacetPool m_TransformationFacets;
        CEntityVector            m_DirtyEntities;
        CEntityDelegates         m_EntityDelegates;
        unsigned int             m_EntityID;

    private:

        void UpdateEntity(CEntity& _rEntity);
        void UpdateActorEntity(CEntity& _rEntity);
        void UpdateLightEntity(CEntity& _rEntity);

        template<typename THasHierarchy>
        void UpdateWorldMatrix(CEntity& _rEntity, THasHierarchy _HasHierarchy);
    };
} // namespace

namespace
{
    CDtLvlEntityManager::CDtLvlEntityManager()
        : m_Entities            ()
        , m_HierarchyFacets     ()
        , m_TransformationFacets()
        , m_DirtyEntities       ()
        , m_EntityDelegates     ()
        , m_EntityID            (0)
    {
        m_DirtyEntities.reserve(65536);
    }
    
    // -----------------------------------------------------------------------------
    
    CDtLvlEntityManager::~CDtLvlEntityManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::Clear()
    {
        m_Entities            .Clear();
        m_HierarchyFacets     .Clear();
        m_TransformationFacets.Clear();

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    CEntity& CDtLvlEntityManager::CreateEntities(const SSceneDescriptor& _rDescriptor)
    {
        assert(_rDescriptor.m_pSceneName != 0);

        SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = SEntityCategory::Actor;
        EntityDesc.m_EntityType     = SActorType::Node;
        EntityDesc.m_FacetFlags     = CEntity::FacetTransformation | CEntity::FacetHierarchy;

        // -----------------------------------------------------------------------------
        // Recursive construction of scene entities
        // -----------------------------------------------------------------------------
        std::function<void(const aiNode*, Dt::CEntity*, const aiScene*)> CreateEntitiesFromScene = [&](const aiNode* _pNode, Dt::CEntity* _pParentEntity, const aiScene* _pScene)
        {
            // -----------------------------------------------------------------------------
            // If we have some mesh data send this mesh to graphic and set facet on entity.
            // This collection of mesh is a model with different surfaces!
            // -----------------------------------------------------------------------------
            if (_pNode->mNumMeshes > 0)
            {
                SModelSceneDescriptor ModelSceneDesc;

                ModelSceneDesc.m_pNode  = _pNode;
                ModelSceneDesc.m_pScene = _pScene;

                CModel& rModel = ModelManager::CreateModel(ModelSceneDesc);

                Dt::CModelActorFacet* pModelActorFacet = ActorManager::CreateModelActor();

                pModelActorFacet->SetModel(&rModel);

                _pParentEntity->SetDetailFacet(SFacetCategory::Data, pModelActorFacet);

                _pParentEntity->SetType(SActorType::Model);
            }

            // -----------------------------------------------------------------------------
            // Check sub entities
            // -----------------------------------------------------------------------------
            unsigned int NumberOfEntities = _pNode->mNumChildren;

            for (unsigned int IndexOfEntity = 0; IndexOfEntity < NumberOfEntities; ++IndexOfEntity)
            {
                aiNode* pChildren = _pNode->mChildren[IndexOfEntity];

                // -----------------------------------------------------------------------------
                // Prepare data
                // -----------------------------------------------------------------------------
                aiVector3D   NodePosition;
                aiVector3D   NodeScale;
                aiQuaternion NodeRotation;

                pChildren->mTransformation.Decompose(NodeScale, NodeRotation, NodePosition);

                // -----------------------------------------------------------------------------
                // Create an child entity
                // -----------------------------------------------------------------------------
                Dt::CEntity& rChildEntity = CreateEntity(EntityDesc);

                Dt::CTransformationFacet* pTransformationFacet = rChildEntity.GetTransformationFacet();

                pTransformationFacet->SetPosition(Base::Float3(NodePosition.x, NodePosition.y, NodePosition.z));
                pTransformationFacet->SetRotation(Base::Float3(NodeRotation.x, NodeRotation.y, NodeRotation.z));
                pTransformationFacet->SetScale   (Base::Float3(NodeScale.x,    NodeScale.y,    NodeScale.z));

                _pParentEntity->Attach(rChildEntity);

                // -----------------------------------------------------------------------------
                // Check this new child on new child and important data like meshes, lights,
                // ...
                // -----------------------------------------------------------------------------
                CreateEntitiesFromScene(pChildren, &rChildEntity, _pScene);
            }
        };

        // -----------------------------------------------------------------------------
        // Build path to scenes in file system
        // -----------------------------------------------------------------------------
		std::string PathToModel = g_PathToAssets + _rDescriptor.m_pSceneName;

        // -----------------------------------------------------------------------------
        // Load file with ASSIMP asset importer
        // -----------------------------------------------------------------------------
        Assimp::Importer Importer;

        unsigned int Flags = aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;

        const aiScene* pScene = Importer.ReadFile(PathToModel.c_str(), Flags);

        if (!pScene)
        {
            BASE_THROWV("Can't load scene file %s; Code: %s", _rDescriptor.m_pSceneName, Importer.GetErrorString());
        }

        // -----------------------------------------------------------------------------
        // Create root node
        // -----------------------------------------------------------------------------
        Dt::CEntity& rRootEntity = CreateEntity(EntityDesc);

        // -----------------------------------------------------------------------------
        // Entities with model data
        // -----------------------------------------------------------------------------
        CreateEntitiesFromScene(pScene->mRootNode, &rRootEntity, pScene);

        return rRootEntity;
    }

    // -----------------------------------------------------------------------------

    CEntity& CDtLvlEntityManager::CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID)
    {
        CInternEntity& rEntity = m_Entities.Allocate();

        CEntity::BID ID = _ID;

        if (ID == CEntity::s_InvalidID)
        {
            ID = m_EntityID;

            ++m_EntityID;
        }
        else
        {
            m_EntityID = Base::Max(ID, m_EntityID);

            ++ m_EntityID;
        }

        rEntity.m_ID               = ID;
        rEntity.m_Flags.m_Category = _rDescriptor.m_EntityCategory;
        rEntity.m_Flags.m_Type     = _rDescriptor.m_EntityType;

        if ((_rDescriptor.m_FacetFlags & CEntity::FacetHierarchy) != 0)
        {
            rEntity.m_pHierarchyFacet = &m_HierarchyFacets.Allocate();
        }

        if ((_rDescriptor.m_FacetFlags & CEntity::FacetTransformation) != 0)
        {
            rEntity.m_pTransformationFacet = &m_TransformationFacets.Allocate();
        }

        return rEntity;
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::FreeEntity(CEntity& _rEntity)
    {
        CInternEntity& rInternEntity = static_cast<CInternEntity&>(_rEntity);

        if (rInternEntity.m_pHierarchyFacet != nullptr)
        {
            m_HierarchyFacets.Free(static_cast<CInternHierarchyFacet*>(rInternEntity.m_pHierarchyFacet));
        }

        if (rInternEntity.m_pTransformationFacet != nullptr)
        {
            m_TransformationFacets.Free(static_cast<CInternTransformationFacet*>(rInternEntity.m_pTransformationFacet));
        }

        m_Entities.Free(&rInternEntity);
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags)
    {
        // -----------------------------------------------------------------------------
        // Set current entity dirty flag
        // -----------------------------------------------------------------------------
        _rEntity.SetDirtyFlags(_DirtyFlags);

        m_DirtyEntities.push_back(&_rEntity);

        // -----------------------------------------------------------------------------
        // Send new dirty entity to all handler
        // -----------------------------------------------------------------------------
        CEntityDelegates::iterator CurrentDirtyEntityDelegate = m_EntityDelegates.begin();
        CEntityDelegates::iterator EndOfDirtyEntityDelegates = m_EntityDelegates.end();

        for (; CurrentDirtyEntityDelegate != EndOfDirtyEntityDelegates; ++CurrentDirtyEntityDelegate)
        {
            (*CurrentDirtyEntityDelegate)(&_rEntity);
        }

        // -----------------------------------------------------------------------------
        // Now: set every child to dirty
        // -----------------------------------------------------------------------------
        CEntity*         pChildEntity;
        CHierarchyFacet* pHierarchyFacet;
        CHierarchyFacet* pChildHierarchyFacet;

        pHierarchyFacet = _rEntity.GetHierarchyFacet();

        if (pHierarchyFacet == nullptr)
        {
            return;
        }

        pChildEntity = pHierarchyFacet->GetFirstChild();

        for (; pChildEntity != nullptr; )
        {
            pChildHierarchyFacet = pChildEntity->GetHierarchyFacet();

            if (pChildHierarchyFacet != nullptr)
            {
                MarkEntityAsDirty(*pChildEntity, _DirtyFlags);

                pChildEntity = pChildHierarchyFacet->GetSibling();
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::Update()
    {
        CEntityVector::iterator CurrentDirtyEntity = m_DirtyEntities.begin();
        CEntityVector::iterator EndOfDirtyEntities = m_DirtyEntities.end();

        for (; CurrentDirtyEntity != EndOfDirtyEntities; ++ CurrentDirtyEntity)
        {
            UpdateEntity(**CurrentDirtyEntity);
        }

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------
    
    void CDtLvlEntityManager::RegisterDirtyEntityHandler(CEntityDelegate _NewDelegate)
    {
        m_EntityDelegates.push_back(_NewDelegate);
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::UpdateEntity(CEntity& _rEntity)
    {
        unsigned int     DirtyFlags;
        CHierarchyFacet* pHierarchicalFacet;

        // -----------------------------------------------------------------------------
        // Update world matrix
        // -----------------------------------------------------------------------------
        pHierarchicalFacet = _rEntity.GetHierarchyFacet();

        if (pHierarchicalFacet != nullptr)
        {
            const Base::U64 TimeStamp = Core::Time::GetNumberOfFrame();

            if (pHierarchicalFacet->GetTimeStamp() == TimeStamp)
            {
                return;
            }

            pHierarchicalFacet->SetTimeStamp(TimeStamp);

            UpdateWorldMatrix(_rEntity, true);
        }
        else
        {
            UpdateWorldMatrix(_rEntity, false);
        }

        // -----------------------------------------------------------------------------
        // Update specific entity categories
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetCategory())
        {
        case SEntityCategory::Actor: UpdateActorEntity(_rEntity); break;
        case SEntityCategory::Light: UpdateLightEntity(_rEntity); break;
        }

        // -----------------------------------------------------------------------------
        // Update entity in map
        // -----------------------------------------------------------------------------
        DirtyFlags = _rEntity.GetDirtyFlags();

        if ((DirtyFlags & CEntity::DirtyAdd) != 0 && (DirtyFlags & CEntity::DirtyRemove) == 0)
        {
            Map::AddEntity(_rEntity);
        }
        else if ((DirtyFlags & CEntity::DirtyRemove) != 0)
        {
            Map::RemoveEntity(_rEntity);

            if ((DirtyFlags & CEntity::DirtyDestroy) != 0)
            {
                FreeEntity(_rEntity);
            }
        }
        else if ((DirtyFlags & CEntity::DirtyMove) != 0)
        {
            Map::MoveEntity(_rEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::UpdateActorEntity(CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);

        // TODO:
        // Update AABB of world
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::UpdateLightEntity(CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);

        // TODO:
        // Update light including AABB
    }

    // -----------------------------------------------------------------------------

    template<typename THasHierarchy>
    void CDtLvlEntityManager::UpdateWorldMatrix(CEntity& _rEntity, THasHierarchy _HasHierarchy)
    {
        CHierarchyFacet*      pHierarchyFacet;
        CTransformationFacet* pTransformationFacet;
        CEntity*              pParentEntity;
        CTransformationFacet* pParentTransformationFacet;

        // -----------------------------------------------------------------------------
        // Check if entity has a transformation facet.
        // -----------------------------------------------------------------------------
        pTransformationFacet = _rEntity.GetTransformationFacet();

        if (pTransformationFacet == nullptr)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Calculate the world matrix of the entity in SRT order.
        // -----------------------------------------------------------------------------
        Base::Float4x4 WorldMatrix;

        WorldMatrix.SetScale(pTransformationFacet->GetScale());

        Base::Float4x4 RotationMatrix;

        const Base::Float3& rRotation = pTransformationFacet->GetRotation();

        RotationMatrix.SetRotation(rRotation[0], rRotation[1], rRotation[2]);

        WorldMatrix *= RotationMatrix;

        // -----------------------------------------------------------------------------
        // Check for an animation hierarchy.
        // -----------------------------------------------------------------------------
        if (_HasHierarchy)
        {
            pHierarchyFacet = _rEntity.GetHierarchyFacet();

            assert(pHierarchyFacet != nullptr);

            pParentEntity = pHierarchyFacet->GetParent();

            // -----------------------------------------------------------------------------
            // In case of an hierarchy we use the position relative to the parent.
            // -----------------------------------------------------------------------------
            WorldMatrix.InjectTranslation(pTransformationFacet->GetPosition());

            if (pParentEntity != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Ensure that the parent world matrix is calculated before the child.
                // -----------------------------------------------------------------------------
                UpdateEntity(*pParentEntity);

                pParentTransformationFacet = pParentEntity->GetTransformationFacet();

                WorldMatrix *= pParentTransformationFacet->GetWorldMatrix();
            }
        }
        else
        {
            // -----------------------------------------------------------------------------
            // If the entity does not have a parent then use the absolute world position.
            // -----------------------------------------------------------------------------
            WorldMatrix.InjectTranslation(_rEntity.GetWorldPosition());
        }

        pTransformationFacet->SetWorldMatrix(WorldMatrix);

        // -----------------------------------------------------------------------------
        // Extract the world space position of the entity from the world matrix.
        // -----------------------------------------------------------------------------
        Base::Float3 WorldPosition;

        WorldMatrix.GetTranslation(WorldPosition);

        _rEntity.SetWorldPosition(WorldPosition);
    }
} // namespace

namespace Dt
{
namespace EntityManager
{
    void OnStart()
    {
        CDtLvlEntityManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtLvlEntityManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtLvlEntityManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CEntity& CreateEntities(const SSceneDescriptor& _rDescriptor)
    {
        return CDtLvlEntityManager::GetInstance().CreateEntities(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID)
    {
        return CDtLvlEntityManager::GetInstance().CreateEntity(_rDescriptor, _ID);
    }

    // -----------------------------------------------------------------------------

    void FreeEntity(CEntity& _rEntity)
    {
        CDtLvlEntityManager::GetInstance().FreeEntity(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags)
    {
        CDtLvlEntityManager::GetInstance().MarkEntityAsDirty(_rEntity, _DirtyFlags);
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CDtLvlEntityManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------
    
    void RegisterDirtyEntityHandler(CEntityDelegate _NewDelegate)
    {
        CDtLvlEntityManager::GetInstance().RegisterDirtyEntityHandler(_NewDelegate);
    }
} // namespace EntityManager
} // namespace Dt
