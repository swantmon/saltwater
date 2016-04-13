//
//  data_entity.h
//  data
//
//  Created by Tobias Schwandt on 05/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb3.h"
#include "base/base_typedef.h"
#include "base/base_vector3.h"

#include "data/data_entity_folder.h"

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
            Actor,
            Light,
            FX,
            NumberOfCategories,
            UndefinedCategory = -1,
        };
    };

    struct SFacetCategory
    {
        enum
        {
            Data,                                           //< General informations about this entity available in the whole project
            Graphic,                                        //< Graphic specific informations like target sets, buffer, ...
            Script,                                         //< Scripting the logical behavior of an entity
            NumberOfCategories,
            UndefinedCategory = -1,
        };
    };

    struct SEntityLayer
    {
        enum
        {
            Default,
            TransparentFX,
            IgnoreRaycast,
            UI,
            NumberOfLayers,
            UndefinedLayer = -1
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
            DirtyDestroy = 0x10,
            DirtyDetail  = 0x20,
        };
        
        enum EFacetFlags
        {
            FacetHierarchy      = 0x01,                      //< Either the entity is inside a hierarchy or not (global entity are not part of scene graph)
            FacetTransformation = 0x02,                      //< Either the entity has a transformation or not (global entity doesn't need transformations)
            FacetDetail         = 0x04,                      //< Detail informations of the entity (light, environment, fx, actors, ...)
        };

    public:

        typedef unsigned int BID;

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
                    unsigned int m_DirtyFlags   : 8;        //< Dirty flags if something happens
                    unsigned int m_Category     : 4;        //< Category of entity (@see SEntitycategories)
                    unsigned int m_Type         : 8;        //< Specific type of the category (e.g.: light -> point, spot, probe, ...)
                    unsigned int m_Layer        : 4;        //< Layer of the entity needed for special culling techniques
                    unsigned int m_IsDynamic    : 1;        //< Either the entity can be moved in game and editor
                    unsigned int m_IsSelectable : 1;        //< Either the entity is selectable in editor
                    unsigned int m_Padding      : 10;
                };

                unsigned int m_Key;
            };
        };

#pragma warning(default : 4201) 

    public:

        BID GetID() const;

        unsigned int GetCategory() const;

        void SetDynamic(bool _Flag);
        bool IsDynamic() const;

        void SetSelectable(bool _Flag);
        bool IsSelectable() const;

        void SetLayer(unsigned int _Layer);
        unsigned int GetLayer() const;

        void SetDirtyFlags(unsigned int _Flags);
        unsigned int GetDirtyFlags() const;

        bool IsInMap() const;

        void SetType(unsigned int _Type);
        unsigned int GetType() const;

        void SetWorldPosition(const Base::Float3& _rPosition);
        Base::Float3& GetWorldPosition();
        const Base::Float3& GetWorldPosition() const;

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

        void SetDetailFacet(unsigned int _Category, void* _pFacet);
        void* GetDetailFacet(unsigned int _Category);
        const void* GetDetailFacet(unsigned int _Category) const;

    public:

        void Attach(CEntity& _rEntity);

    protected:
        
        Dt::CEntity*          m_pNextNeighbor;               //< Next neighbor entity in folder
        Dt::CEntity*          m_pPreviousNeighbor;           //< Previous neighbor entity in folder
        Dt::CEntityFolder*    m_pFolder;                     //< Pointer to folder of this entity
        CHierarchyFacet*      m_pHierarchyFacet;             //< Contains hierarchical informations of the entity (scene graph)
        CTransformationFacet* m_pTransformationFacet;        //< Contains transformation informations depending on hierarchy
        void*                 m_pDetailFacets[SFacetCategory::NumberOfCategories];                //< Contains detail informations of the entity (@see EFacetFlags::FacetDetail)
        BID                   m_ID;                          //< A specific unique id of this entity inside the map
        Base::AABB3Float      m_WorldAABB;                   //< Axis Aligned Bounding Box (AABB) of the entity in map for region bounding box calculations
        Base::Float3          m_WorldPosition;               //< World position of the entity
        SFlags                m_Flags;                       //< Set of possible states of the entity (dirty, type, level, ... -> @see SFlags)

    protected:

        CEntity();
        ~CEntity();
    };
} // namespace Dt