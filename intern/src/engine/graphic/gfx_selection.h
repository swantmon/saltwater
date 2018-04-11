#pragma once

#include "base/base_include_glm.h"

namespace Gfx
{
    struct SPickFlag
    {
        enum Enum
        {
            Nothing    = 0x00,
            Actor      = 0x01,
            AR         = 0x02,
            Gizmo      = 0x04,
            Everything = 0xF
        };
    };
} // namespace Gfx


namespace Gfx
{
    struct SHitFlag
    {
        enum Enum
        {
            Nothing,
            Entity,
            Gizmo,
        };
    };
} // namespace Gfx

namespace Gfx
{
    class CSelectionTicket
    {
    public:

        SHitFlag::Enum m_HitFlag;
        glm::vec3      m_WSPosition;
        glm::vec3      m_WSNormal;
        float          m_Depth;
        void*          m_pObject;
    };
} // namespace Gfx