//
//  data_entity_manager.h
//  data
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//


#pragma once

#include "data/data_entity.h"

#include <functional>
#include <string>
#include <vector>

namespace Dt
{
    class CMesh;
} // namespace Dt

namespace Dt
{
namespace EntityManager
{
    typedef std::function<void(Dt::CEntity* _pEntity)> CEntityDelegate;
} // namespace EntityManager
} // namespace Dt

#define DATA_DIRTY_ENTITY_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)

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
namespace EntityManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID = CEntity::s_InvalidID);

    std::vector<CEntity*> CreateEntitiesFromScene(const std::string& _rFile);

    void FreeEntity(CEntity& _rEntity);

    CEntity* GetEntityByID(CEntity::BID _ID);

    void MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags);

    void Update();

    void RegisterDirtyEntityHandler(CEntityDelegate _NewDelegate);
} // namespace EntityManager
} // namespace Dt
