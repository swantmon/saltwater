#pragma once

#include "base/base_vector3.h"

namespace Gfx
{
    struct SPickFlag
    {
        enum Enum
        {
            Nothing = 0x00,
            Actor   = 0x01,
            AR      = 0x02,
            Gizmo   = 0x04,
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
        Base::Float3   m_WSPosition;
        Base::Float3   m_WSNormal;
        float          m_Depth;
        void*          m_pObject;
    };
} // namespace Gfx