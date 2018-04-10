
#pragma once

#include "engine/engine_config.h"

#include "base/base_aabb3.h"
#include "base/base_typedef.h"

#include "data/data_entity_folder.h"

#include <string>

namespace Dt
{
    class CHierarchyFacet;
    class CTransformationFacet;
    class CComponentFacet;
    class IComponent;
} // namespace Dt

namespace Dt
{
    struct SEntityCategory
    {
        enum
        {
            Static,
            Dynamic,
            NumberOfCategories,
            UndefinedCategory = -1,
        };
    };

    struct SEntityLayer
    {
        enum
        {
            Default         = 0x00,
            AR              = 0x01,
            TransparentFX   = 0x02,
            IgnoreRaycast   = 0x04,
            Water           = 0x08,
            UI              = 0x10,
            NumberOfLayers  = 6,
            UndefinedLayer  = -1
        };
    };
} // namespace Dt

namespace Dt
{
    class ENGINE_API CEntity
    {
    public:

        enum EDirtyFlags
        {
            DirtyCreate    = 0x01,
            DirtyAdd       = 0x02,
            DirtyMove      = 0x04,
            DirtyComponent = 0x08,
            DirtyRemove    = 0x10,
            DirtyDestroy   = 0x20
        };
        
        enum EFacetFlags
        {
            FacetHierarchy      = 0x01,                 //< Either the entity is inside a hierarchy or not (global entities are not part of scene graph)
            FacetTransformation = 0x02,                 //< Either the entity has a transformation or not (global entities doesn't need transformations)
            FacetComponents     = 0x04,                 //< Either the entity has components or not (node/empty entities doesn't need components)
        };

    public:

        typedef Base::ID BID;

    public:

        static const BID s_InvalidID = static_cast<BID>(-1);

    public:

#pragma warning(push)
#pragma warning(disable : 4201)

        struct SFlags
        {
            union 
            {
                struct 
                {
                    unsigned int m_DirtyFlags   : 8;        //< Dirty flags if something happens
                    unsigned int m_Category     : 4;        //< Category of entity (@see SEntitycategories)
                    unsigned int m_Layer        : 8;        //< Layer of the entity needed for special culling techniques
                    unsigned int m_IsDynamic    : 1;        //< Either the entity can be moved
                    unsigned int m_IsSelectable : 1;        //< Either the entity is selectable
                    unsigned int m_IsActive     : 1;        //< Either the entity is active
                    unsigned int m_Padding      : 9;
                };

                unsigned int m_Key;
            };
        };

#pragma warning(pop) 

    public:

        BID GetID() const;

        void SetCategory(unsigned int _Category);
        unsigned int GetCategory() const;

        void SetName(const std::string& _rName);
        const std::string& GetName() const;

        void SetDynamic(bool _Flag);
        bool IsDynamic() const;

        void SetSelectable(bool _Flag);
        bool IsSelectable() const;

        void SetActive(bool _Flag);
        bool IsActive() const;

        void SetLayer(unsigned int _Layer);
        unsigned int GetLayer() const;

        void SetDirtyFlags(unsigned int _Flags);
        unsigned int GetDirtyFlags() const;

        bool IsInMap() const;

        void SetWorldPosition(const glm::vec3& _rPosition);
        glm::vec3& GetWorldPosition();
        const glm::vec3& GetWorldPosition() const;

        void SetWorldAABB(const Base::AABB3Float& _rAABB);
        Base::AABB3Float& GetWorldAABB();
        const Base::AABB3Float& GetWorldAABB() const;

        void SetFolder(CEntityFolder* _pFolder);
        CEntityFolder* GetFolder();
        const CEntityFolder* GetFolder() const;

        void SetNext(CEntity* _pLink);
        CEntity* GetNext();
        const CEntity* GetNext() const;

        void SetPrevious(CEntity* _pLink);
        CEntity* GetPrevious();
        const CEntity* GetPrevious() const;

        void SetHierarchyFacet(CHierarchyFacet* _pFacet);
        CHierarchyFacet* GetHierarchyFacet();
        const CHierarchyFacet* GetHierarchyFacet() const;

        void SetTransformationFacet(CTransformationFacet* _pFacet);
        CTransformationFacet* GetTransformationFacet();
        const CTransformationFacet* GetTransformationFacet() const;

        void SetComponentFacet(CComponentFacet* _pFacet);
        CComponentFacet* GetComponentFacet();
        const CComponentFacet* GetComponentFacet() const;

    public:

        void Attach(CEntity& _rEntity);
        void Detach();

        void AttachComponent(Dt::IComponent* _pComponent);
        void DetachComponent(Dt::IComponent* _pComponent);

    protected:
        
        Dt::CEntity*          m_pNextNeighbor;                                                    //< Next neighbor entity in folder
        Dt::CEntity*          m_pPreviousNeighbor;                                                //< Previous neighbor entity in folder
        Dt::CEntityFolder*    m_pFolder;                                                          //< Pointer to folder of this entity
        CHierarchyFacet*      m_pHierarchyFacet;                                                  //< Contains hierarchical information of the entity (scene graph)
        CTransformationFacet* m_pTransformationFacet;                                             //< Contains transformation information depending on hierarchy
        CComponentFacet*      m_pComponentsFacet;                                                 //< Contains components of this entity
        BID                   m_ID;                                                               //< A specific unique id of this entity inside the map
        std::string           m_Name;                                                             //< A name of the entity to search for inside scripts
        Base::AABB3Float      m_WorldAABB;                                                        //< Axis Aligned Bounding Box (AABB) of the entity in map for region bounding box calculations
        glm::vec3             m_WorldPosition;                                                    //< World position of the entity
        SFlags                m_Flags;                                                            //< Set of possible states of the entity (dirty, type, level, ... -> @see SFlags)

    protected:

        CEntity();
        ~CEntity();
    };
} // namespace Dt