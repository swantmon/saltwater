
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_component_manager.h"

namespace Gfx
{
    CComponentManager::CComponentManager()
        : m_Components()
    {

    }

    // -----------------------------------------------------------------------------

    CComponentManager::~CComponentManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Clear()
    {
        m_Components.clear();
    }
} // namespace Gfx