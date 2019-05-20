
#pragma once

#include "base/base_delegate.h"
#include "base/base_pool.h"

#include "engine/engine_config.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_hierarchy_facet.h"
#include "engine/data/data_map.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_transformation_facet.h"

#include <functional>
#include <string>
#include <vector>

namespace Dt
{
    class CMesh;
} // namespace Dt

namespace Dt
{
    struct SEntityDescriptor
    {
        unsigned int m_EntityCategory;
        unsigned int m_FacetFlags;
    };
} // namespace Dt

namespace Dt
{
    class ENGINE_API CEntityManager : private Base::CUncopyable
    {
    public:

        using CEntityDelegate = Base::CDelegate<Dt::CEntity*>;

    public:

        static CEntityManager& GetInstance();

    public:

        CEntityManager();
        ~CEntityManager();

    public:

        void OnStart();
        void OnExit();

        void Clear();

        CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID = CEntity::s_InvalidID);

        std::vector<CEntity*> CreateEntitiesFromScene(const std::string& _rFile);

        void FreeEntity(CEntity& _rEntity);

        CEntity* GetEntityByID(CEntity::BID _ID);

        void MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags);

        void Update();

        CEntityDelegate::HandleType RegisterDirtyEntityHandler(CEntityDelegate::FunctionType _Function);

    private:

        class CInternEntity : public CEntity
        {
        private:
            friend class CEntityManager;
        };

        class CInternHierarchyFacet : public CHierarchyFacet
        {
        private:
            friend class CEntityManager;
        };

        class CInternTransformationFacet : public CTransformationFacet
        {
        private:
            friend class CEntityManager;
        };

        class CInternComponentsFacet : public CComponentFacet
        {
        private:
            friend class CEntityManager;
        };

    private:

        using CEntityPool              = Base::CPool<CInternEntity, 2048>;
        using CHierarchyFacetPool      = Base::CPool<CInternHierarchyFacet, 2048>;
        using CTransformationFacetPool = Base::CPool<CInternTransformationFacet, 2048>;
        using CComponentsFacetPool     = Base::CPool<CInternComponentsFacet, 2048>;

        using CEntityByIDs    = std::unordered_map<Base::ID, CInternEntity*>;
        using CEntityByIDPair = CEntityByIDs::iterator;

    private:

        CEntityPool              m_Entities;
        CHierarchyFacetPool      m_HierarchyFacets;
        CTransformationFacetPool m_TransformationFacets;
        CComponentsFacetPool     m_ComponentsFacets;
        CEntityByIDs             m_EntityByID;
        Base::ID                 m_EntityID;

    private:

        CEntityDelegate m_EntityDelegate;

        void UpdateEntity(CEntity& _rEntity);

        template<typename THasHierarchy>
        void UpdateWorldMatrix(CEntity& _rEntity, THasHierarchy _HasHierarchy);
    };
} // namespace Dt
