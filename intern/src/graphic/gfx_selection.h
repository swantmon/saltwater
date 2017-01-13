#pragma once

#include "base/base_vector3.h"

namespace Gfx
{
    struct SHitFlag
    {
        enum Enum
        {
            Nothing,
            Entity,
            Terrain,
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