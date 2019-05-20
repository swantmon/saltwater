
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_asset_importer.h"
#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"
#include "engine/core/core_time.h"

#include "engine/data/data_entity_manager.h"

#include "engine/graphic/gfx_material_manager.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <assert.h>
#include <unordered_map>

namespace Dt
{
    CEntityManager::CEntityManager()
        : m_Entities            ()
        , m_HierarchyFacets     ()
        , m_TransformationFacets()
        , m_EntityByID          ()
        , m_EntityID            (0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CEntityManager::~CEntityManager()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    CEntityManager& CEntityManager::GetInstance()
    {
        static CEntityManager s_Instance;
        return s_Instance;
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::Clear()
    {
        m_Entities            .Clear();
        m_HierarchyFacets     .Clear();
        m_TransformationFacets.Clear();
        m_ComponentsFacets    .Clear();

        m_EntityByID.clear();
    }

    // -----------------------------------------------------------------------------

    CEntity& CEntityManager::CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID)
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

    std::vector<CEntity*> CEntityManager::CreateEntitiesFromScene(const std::string& _rFile)
    {
        // -----------------------------------------------------------------------------
        // Create an vector of new entities
        // -----------------------------------------------------------------------------
        std::vector<CEntity*> VectorOfEntites;

        // -----------------------------------------------------------------------------
        // Build path to texture in file system and load scene
        // -----------------------------------------------------------------------------
        int GeneratorFlag = Core::AssetGenerator::SGeneratorFlag::Default;

        std::string PathToModel = Core::AssetManager::GetPathToAssets() + "/" + _rFile;

        auto Importer = Core::AssetImporter::AllocateAssimpImporter(PathToModel, GeneratorFlag);

        if (Importer == nullptr) return VectorOfEntites;

        const auto* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pImporter == nullptr) return VectorOfEntites;

        const aiScene* pScene = pImporter->GetScene();

        if (!pScene) return VectorOfEntites;

        // -----------------------------------------------------------------------------
        // Create a new entity for each mesh
        // -----------------------------------------------------------------------------
        Dt::SEntityDescriptor EntityDescriptor;

        EntityDescriptor.m_EntityCategory = Dt::SEntityCategory::Dynamic;
        EntityDescriptor.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        auto NumberOfMeshes = static_cast<int>(pScene->mNumMeshes);

        for (int IndexOfMesh = 0; IndexOfMesh < NumberOfMeshes; ++IndexOfMesh)
        {
            aiMesh* pMesh = pScene->mMeshes[IndexOfMesh];

            auto& rChildEntity = static_cast<CInternEntity&>(CreateEntity(EntityDescriptor));

            rChildEntity.SetName(pMesh->mName.C_Str());

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<CMeshComponent>();

            pMeshComponent->SetFilename(_rFile);
            pMeshComponent->SetGeneratorFlag(GeneratorFlag);
            pMeshComponent->SetMeshType(CMeshComponent::Asset);
            pMeshComponent->SetMeshIndex(IndexOfMesh);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, CMeshComponent::DirtyCreate);

            rChildEntity.AttachComponent(pMeshComponent);

            // -----------------------------------------------------------------------------
            // Material
            // -----------------------------------------------------------------------------
            if (pMesh->mMaterialIndex < pScene->mNumMaterials)
            {
                auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

                Gfx::MaterialManager::CreateMaterialFromAssimp(_rFile, pMesh->mMaterialIndex, pMaterialComponent);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

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

    void CEntityManager::FreeEntity(CEntity& _rEntity)
    {
        auto& rInternEntity = static_cast<CInternEntity&>(_rEntity);

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

    CEntity* CEntityManager::GetEntityByID(CEntity::BID _ID)
    {
        if (m_EntityByID.find(_ID) == m_EntityByID.end()) return nullptr;

        return m_EntityByID.at(_ID);
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags)
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
        m_EntityDelegate.Notify(&_rEntity);

        _rEntity.SetDirtyFlags(0);
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::Update()
    {
    }

    // -----------------------------------------------------------------------------
    
    CEntityManager::CEntityDelegate::HandleType CEntityManager::RegisterDirtyEntityHandler(CEntityDelegate::FunctionType _Function)
    {
        return m_EntityDelegate.Register(_Function);
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::Read(Base::CTextReader& _rCodec)
    {
        int NumberOfEntities = 0;

        _rCodec >> NumberOfEntities;

        for (int i = 0; i < NumberOfEntities; ++i)
        {
            auto CurrentEntity = m_Entities.Allocate();

            _rCodec >> CurrentEntity;
        }

        bool Check = false;

        for (auto CurrentEntity = m_Entities.Begin(); CurrentEntity != m_Entities.End(); ++CurrentEntity)
        {
            _rCodec >> Check;
            if (Check)
            {
                CurrentEntity->m_pTransformationFacet = &m_TransformationFacets.Allocate();

                _rCodec >> CurrentEntity->m_pTransformationFacet;
            }

            _rCodec >> Check;
            if (Check)
            {
                CurrentEntity->m_pHierarchyFacet = &m_HierarchyFacets.Allocate();

                _rCodec >> CurrentEntity->m_pHierarchyFacet;
            }

            _rCodec >> Check;
            if (Check)
            {
                CurrentEntity->m_pComponentsFacet = &m_ComponentsFacets.Allocate();

                _rCodec >> CurrentEntity->m_pComponentsFacet;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::Write(Base::CTextWriter& _rCodec)
    {
        unsigned int NumberOfEntities = 0;

        NumberOfEntities = m_Entities.GetNumberOfItems();

        _rCodec << NumberOfEntities;

        for (auto CurrentEntity = m_Entities.Begin(); CurrentEntity != m_Entities.End(); ++CurrentEntity)
        {
            _rCodec << *CurrentEntity;
        }

        bool Check = false;

        for (auto CurrentEntity = m_Entities.Begin(); CurrentEntity != m_Entities.End(); ++CurrentEntity)
        {
            auto pFacetTransformation = CurrentEntity->GetTransformationFacet();
            Check = pFacetTransformation != nullptr;
            _rCodec << Check;
            if (Check) _rCodec << *pFacetTransformation;

            auto pFacetHierarchy = CurrentEntity->GetHierarchyFacet();
            Check = pFacetHierarchy != nullptr;
            _rCodec << Check;
            if (Check) _rCodec << *pFacetHierarchy;

            auto pFacetComponent = CurrentEntity->GetComponentFacet();
            Check = pFacetComponent != nullptr;
            _rCodec << Check;
            if (Check) _rCodec << *pFacetComponent;
        }
    }

    // -----------------------------------------------------------------------------

    void CEntityManager::UpdateEntity(CEntity& _rEntity)
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
    void CEntityManager::UpdateWorldMatrix(CEntity& _rEntity, THasHierarchy _HasHierarchy)
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
} // namespace Dt