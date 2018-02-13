
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
        for (auto Element : m_Components)
        {
            Base::CMemory::DeleteObject(&Element);
        }

        m_Components.clear();
    }
} // namespace Gfx