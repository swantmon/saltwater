
#pragma once

#include "base/base_aabb3.h"
#include "base/base_vector3.h"

#include "data/data_entity.h"
#include "data/data_entity_folder.h"

namespace Dt
{
    class CRegion
    {
    public:

        typedef unsigned int BID;

    public:

        static const unsigned int s_NumberOfCategories = SEntityCategory::NumberOfCategories;
        static const unsigned int s_NumberOfMetersX    = 32;
        static const unsigned int s_NumberOfMetersY    = 32;

    public:

        BID              m_ID;
        Base::Int3       m_Position;
        Base::AABB3Float m_AABB;
        CEntityFolder    m_EntityFolders[s_NumberOfCategories];
    };
} // namespace Dt