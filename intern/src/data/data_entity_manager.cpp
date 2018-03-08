
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_importer.h"
#include "core/core_asset_manager.h"
#include "core/core_time.h"

#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_component.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_component.h"
#include "data/data_transformation_facet.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <assert.h>
#include <unordered_map>
#include <vector>

using namespace Dt;
using namespace Dt::EntityManager;

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

        CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID = CEntity::s_InvalidID);

        std::vector<CEntity*> CreateEntitiesFromScene(const std::string& _rFile);

        void FreeEntity(CEntity& _rEntity);

        CEntity& GetEntityByID(CEntity::BID _ID);

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

        class CInternComponentsFacet : public CComponentFacet
        {
        private:
            friend class CDtLvlEntityManager;
        };
        
    private:
        
        typedef Base::CPool<CInternEntity, 2048>              CEntityPool;
        typedef Base::CPool<CInternHierarchyFacet, 2048>      CHierarchyFacetPool;
        typedef Base::CPool<CInternTransformationFacet, 2048> CTransformationFacetPool;
        typedef Base::CPool<CInternComponentsFacet, 2048>     CComponentsFacetPool;
        typedef std::vector<CEntityDelegate>                  CEntityDelegates;

        typedef std::unordered_map<Base::ID, CInternEntity*> CEntityByIDs;
        typedef CEntityByIDs::iterator                       CEntityByIDPair;

    private:
        
        CEntityPool              m_Entities;
        CHierarchyFacetPool      m_HierarchyFacets;
        CTransformationFacetPool m_TransformationFacets;
        CComponentsFacetPool     m_ComponentsFacets;
        CEntityDelegates         m_EntityDelegates;
        CEntityByIDs             m_EntityByID;
        Base::ID                 m_EntityID;

    private:

        void UpdateEntity(CEntity& _rEntity);

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
        , m_EntityDelegates     ()
        , m_EntityByID          ()
        , m_EntityID            (0)
    {
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
        m_ComponentsFacets    .Clear();

        m_EntityByID.clear();
    }

    // -----------------------------------------------------------------------------

    CEntity& CDtLvlEntityManager::CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID)
    {
        CInternEntity& rEntity = m_Entities.Allocate();

        CEntity::BID ID = _ID;

        if (ID == CEntity::s_InvalidID)
        {
            ID = m_EntityID;
        }
        else
        {
            m_EntityID = glm::max(ID, m_EntityID);
        }

        ++m_EntityID;

        rEntity.m_ID               = ID;
        rEntity.m_Flags.m_Category = _rDescriptor.m_EntityCategory;

        if ((_rDescriptor.m_FacetFlags & CEntity::FacetHierarchy) != 0)
        {
            rEntity.m_pHierarchyFacet = &m_HierarchyFacets.Allocate();
        }

        if ((_rDescriptor.m_FacetFlags & CEntity::FacetTransformation) != 0)
        {
            rEntity.m_pTransformationFacet = &m_TransformationFacets.Allocate();
        }

        if ((_rDescriptor.m_FacetFlags & CEntity::FacetComponents) != 0)
        {
            rEntity.m_pComponentsFacet = &m_ComponentsFacets.Allocate();
        }

        m_EntityByID[rEntity.m_ID] = &rEntity;

        return rEntity;
    }

    // -----------------------------------------------------------------------------

    std::vector<CEntity*> CDtLvlEntityManager::CreateEntitiesFromScene(const std::string& _rFile)
    {
        // -----------------------------------------------------------------------------
        // Create an vector of new entities
        // -----------------------------------------------------------------------------
        std::vector<CEntity*> VectorOfEntites;

        // -----------------------------------------------------------------------------
        // Build path to texture in file system and load scene
        // -----------------------------------------------------------------------------
        int GeneratorFlag = Core::AssetGenerator::SGeneratorFlag::Default;

        auto Importer = Core::AssetImporter::AllocateAssimpImporter(_rFile, GeneratorFlag);

        const Assimp::Importer* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        const aiScene* pScene = pImporter->GetScene();

        if (!pScene)
        {
            return VectorOfEntites;
        }

        // -----------------------------------------------------------------------------
        // Create a new entity for each mesh
        // -----------------------------------------------------------------------------
        Dt::SEntityDescriptor EntityDescriptor;

        EntityDescriptor.m_EntityCategory = Dt::SEntityCategory::Dynamic;
        EntityDescriptor.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        int NumberOfMeshes = static_cast<int>(pScene->mNumMeshes);

        for (int IndexOfMesh = 0; IndexOfMesh < NumberOfMeshes; ++IndexOfMesh)
        {
            aiMesh* pMesh = pScene->mMeshes[IndexOfMesh];

            CInternEntity& rChildEntity = static_cast<CInternEntity&>(CreateEntity(EntityDescriptor));

            rChildEntity.SetName(pMesh->mName.C_Str());

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<CMeshComponent>();

            pMeshComponent->SetFilename(_rFile);
            pMeshComponent->SetGeneratorFlag(GeneratorFlag);
            pMeshComponent->SetMeshType(CMeshComponent::Asset);
            pMeshComponent->SetMeshIndex(IndexOfMesh);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pMeshComponent, CMeshComponent::DirtyCreate);

            rChildEntity.AttachComponent(pMeshComponent);

            // -----------------------------------------------------------------------------
            // Material
            // -----------------------------------------------------------------------------
            if (pMesh->mMaterialIndex < pScene->mNumMaterials)
            {
                auto pMaterial = Dt::MaterialManager::CreateMaterialFromAssimp(_rFile, pMesh->mMaterialIndex);

                auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

                pMaterialComponent->SetMaterial(pMaterial);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

                rChildEntity.AttachComponent(pMaterialComponent);
            }

            VectorOfEntites.push_back(&rChildEntity);
        }

        // -----------------------------------------------------------------------------
        // Release importer
        // -----------------------------------------------------------------------------
        Core::AssetImporter::ReleaseImporter(Importer);

        return VectorOfEntites;
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

        if (rInternEntity.m_pComponentsFacet != nullptr)
        {
            m_ComponentsFacets.Free(static_cast<CInternComponentsFacet*>(rInternEntity.m_pComponentsFacet));
        }

        m_Entities.Free(&rInternEntity);
    }

    // -----------------------------------------------------------------------------

    CEntity& CDtLvlEntityManager::GetEntityByID(CEntity::BID _ID)
    {
        return *m_EntityByID[_ID];
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags)
    {
        // -----------------------------------------------------------------------------
        // Set current entity dirty flag
        // -----------------------------------------------------------------------------
        _rEntity.SetDirtyFlags(_DirtyFlags);

        // -----------------------------------------------------------------------------
        // Add entity to map
        // -----------------------------------------------------------------------------
        if ((_DirtyFlags & CEntity::DirtyAdd) != 0 && (_DirtyFlags & CEntity::DirtyRemove) == 0)
        {
            Map::AddEntity(_rEntity);
        }

        // -----------------------------------------------------------------------------
        // Set every child to dirty
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

        // -----------------------------------------------------------------------------
        // Internal Update
        // -----------------------------------------------------------------------------
        UpdateEntity(_rEntity);

        // -----------------------------------------------------------------------------
        // Send new dirty entity to all handler
        // -----------------------------------------------------------------------------
        for (auto& rEntityDelegate : m_EntityDelegates)
        {
            (rEntityDelegate)(&_rEntity);
        }

        _rEntity.SetDirtyFlags(0);
    }

    // -----------------------------------------------------------------------------

    void CDtLvlEntityManager::Update()
    {
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
        // Update entity in map
        // -----------------------------------------------------------------------------
        DirtyFlags = _rEntity.GetDirtyFlags();

        if ((DirtyFlags & CEntity::DirtyRemove) != 0)
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
        glm::mat4 WorldMatrix;

        WorldMatrix = glm::scale(pTransformationFacet->GetScale());

        glm::mat4 RotationMatrix;

        RotationMatrix = glm::toMat4(pTransformationFacet->GetRotation());

        WorldMatrix = RotationMatrix * WorldMatrix;

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
            WorldMatrix[3] = glm::vec4(pTransformationFacet->GetPosition(), 1.0f);

            if (pParentEntity != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Ensure that the parent world matrix is calculated before the child.
                // -----------------------------------------------------------------------------
                UpdateEntity(*pParentEntity);

                pParentTransformationFacet = pParentEntity->GetTransformationFacet();

                WorldMatrix = pParentTransformationFacet->GetWorldMatrix() * WorldMatrix;
            }
        }
        else
        {
            // -----------------------------------------------------------------------------
            // If the entity does not have a parent then use the absolute world position.
            // -----------------------------------------------------------------------------
            WorldMatrix[3] = glm::vec4(_rEntity.GetWorldPosition(), 1.0f);
        }

        pTransformationFacet->SetWorldMatrix(WorldMatrix);

        // -----------------------------------------------------------------------------
        // Extract the world space position of the entity from the world matrix.
        // -----------------------------------------------------------------------------
        glm::vec3 WorldPosition;

        WorldPosition = WorldMatrix[3];

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

    CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID)
    {
        return CDtLvlEntityManager::GetInstance().CreateEntity(_rDescriptor, _ID);
    }

    // -----------------------------------------------------------------------------

    std::vector<CEntity*> CreateEntitiesFromScene(const std::string& _rFile)
    {
        return CDtLvlEntityManager::GetInstance().CreateEntitiesFromScene(_rFile);
    }

    // -----------------------------------------------------------------------------

    void FreeEntity(CEntity& _rEntity)
    {
        CDtLvlEntityManager::GetInstance().FreeEntity(_rEntity);
    }

    // -----------------------------------------------------------------------------

    CEntity& GetEntityByID(CEntity::BID _ID)
    {
        return CDtLvlEntityManager::GetInstance().GetEntityByID(_ID);
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
