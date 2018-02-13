
#pragma once

#include "base/base_aabb3.h"
#include "base/base_typedef.h"

#include "data/data_component.h"
#include "data/data_entity_folder.h"

#include <string>
#include <vector>

namespace Dt
{
    class CHierarchyFacet;
    class CTransformationFacet;
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
    class CEntity
    {
    public:

        enum EDirtyFlags
        {
            DirtyCreate  = 0x01,
            DirtyAdd     = 0x02,
            DirtyMove    = 0x04,
            DirtyRemove  = 0x08,
            DirtyDestroy = 0x10
        };
        
        enum EFacetFlags
        {
            FacetHierarchy      = 0x01,                 //< Either the entity is inside a hierarchy or not (global entity are not part of scene graph)
            FacetTransformation = 0x02,                 //< Either the entity has a transformation or not (global entity doesn't need transformations)
        };

    public:

        typedef Base::ID BID;

    public:

        static const BID s_InvalidID = static_cast<BID>(-1);

    public:

#pragma warning(disable : 4201)

        struct SFlags
        {
            union 
            {
                struct 
                {
                    unsigned int m_DirtyFlags   :  8;        //< Dirty flags if something happens
                    unsigned int m_Category     :  4;        //< Category of entity (@see SEntitycategories)
                    unsigned int m_Layer        :  8;        //< Layer of the entity needed for special culling techniques
                    unsigned int m_IsDynamic    :  1;        //< Either the entity can be moved in game and editor
                    unsigned int m_IsSelectable :  1;        //< Either the entity is selectable in editor
                    unsigned int m_Padding      : 10;
                };

                unsigned int m_Key;
            };
        };

#pragma warning(default : 4201) 

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

    public:

        void Attach(CEntity& _rEntity);
        void Detach();

    public:

        typedef std::vector<Dt::IComponent*> CComponentVector;

        CComponentVector m_Components; //< Components added to this entity

        template<class T>
        void AddComponent(T& _rComponent);

        template<class T>
        T* GetComponent();

        template<class T>
        const T* GetComponent() const;

        template<class T>
        bool HasComponent() const;

    protected:
        
        Dt::CEntity*          m_pNextNeighbor;                                                    //< Next neighbor entity in folder
        Dt::CEntity*          m_pPreviousNeighbor;                                                //< Previous neighbor entity in folder
        Dt::CEntityFolder*    m_pFolder;                                                          //< Pointer to folder of this entity
        CHierarchyFacet*      m_pHierarchyFacet;                                                  //< Contains hierarchical informations of the entity (scene graph)
        CTransformationFacet* m_pTransformationFacet;                                             //< Contains transformation informations depending on hierarchy
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

namespace Dt
{
    template<class T>
    void CEntity::AddComponent(T& _rComponent)
    {
        _rComponent.SetLinkedEntity(this);

        m_Components.push_back(&_rComponent);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    T* CEntity::GetComponent()
    {
        for (auto Component : m_Components)
        {
            if (Component->GetTypeID() == Base::CTypeInfo::GetTypeID<T>())
            {
                return static_cast<T*>(Component);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const T* CEntity::GetComponent() const
    {
        for (auto Component : m_Components)
        {
            if (Component->GetTypeID() == Base::CTypeInfo::GetTypeID<T>())
            {
                return static_cast<T*>(Component);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CEntity::HasComponent() const
    {
        return GetComponent<T>() != nullptr;
    }
} // namespace Dt