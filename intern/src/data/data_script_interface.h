
#pragma once

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_transformation_facet.h"

#include <string>

Core::Lua::CStaticFunctionList DataEntityObjFuncs;

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(DataEntityObjFuncs, GetID)
{
    Dt::CEntity& rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushUInt(_State, rEntity.GetID());

    return 1;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, SetPosition)
{
    Dt::CEntity&  rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

    if (pTransformationFacet != nullptr)
    {
        pTransformationFacet->SetPosition(rVector);

        Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyMove);
    }

    return 0;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, GetPosition)
{
    Dt::CEntity&  rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult.SetZero();

    Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

    if (pTransformationFacet != nullptr)
    {
        rResult = pTransformationFacet->GetPosition();

        return 1;
    }

    return 0;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, SetRotation)
{
    Dt::CEntity&  rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();
    

    if (pTransformationFacet != nullptr)
    {
        pTransformationFacet->SetRotation(rVector);

        Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyMove);
    }

    return 0;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, GetRotation)
{
    Dt::CEntity&  rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult.SetZero();

    Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

    if (pTransformationFacet != nullptr)
    {
        rResult = pTransformationFacet->GetRotation();

        return 1;
    }

    return 0;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, SetScale)
{
    Dt::CEntity&  rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

    if (pTransformationFacet != nullptr)
    {
        pTransformationFacet->SetScale(rVector);

        Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyMove);
    }

    return 0;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, GetScale)
{
    Dt::CEntity&  rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult.SetZero();

    Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

    if (pTransformationFacet != nullptr)
    {
        rResult = pTransformationFacet->GetScale();

        return 1;
    }

    return 0;
}

LUA_DEFINE_FUNCTION(DataEntityObjFuncs, __tostring)
{
    Dt::CEntity& rEntity = *static_cast<Dt::CEntity*>(Core::Lua::State::GetUserData(_State, 1));

    std::string String;

    String = "Entity (" + std::to_string(rEntity.GetID()) + ")";

    Core::Lua::State::PushString(_State, String.c_str());

    return 1;
}