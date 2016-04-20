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

namespace Dt
{
    class CEntity;
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
    struct SSceneDescriptor
    {
        const Base::Char* m_pSceneName;
    };

    struct SEntityDescriptor
    {
        unsigned int m_EntityCategory;
        unsigned int m_EntityType;
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

    CEntity& CreateEntities(const SSceneDescriptor& _rDescriptor);

    CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID = CEntity::s_InvalidID);

    void FreeEntity(CEntity& _rEntity);

    void MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags);

    void Update();

    void RegisterDirtyEntityHandler(CEntityDelegate _NewDelegate);
} // namespace EntityManager
} // namespace Dt
