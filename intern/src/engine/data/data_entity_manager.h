
#pragma once

#include "base/base_delegate.h"

#include "engine/engine_config.h"

#include "engine/data/data_entity.h"

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
namespace EntityManager
{
    using CEntityDelegate = Base::CDelegate<Dt::CEntity*>;

    ENGINE_API void OnStart();
    ENGINE_API void OnExit();

    ENGINE_API void Update();

    ENGINE_API void Clear();

    ENGINE_API CEntity& CreateEntity(const SEntityDescriptor& _rDescriptor, CEntity::BID _ID = CEntity::s_InvalidID);

    ENGINE_API std::vector<CEntity*> CreateEntitiesFromScene(const std::string& _rFile);

    ENGINE_API void FreeEntity(CEntity& _rEntity);

    ENGINE_API CEntity* GetEntityByID(CEntity::BID _ID);

    ENGINE_API void MarkEntityAsDirty(CEntity& _rEntity, unsigned int _DirtyFlags);

    ENGINE_API CEntityDelegate::HandleType RegisterDirtyEntityHandler(CEntityDelegate::FunctionType _Function);
} // namespace EntityManager
} // namespace Dt
