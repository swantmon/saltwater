
#pragma once

#include "base/base_string.h"
#include "base/base_vector2.h"

namespace Dt
{
    class CARTrackedObjectPluginFacet
    {
    public:

        void SetUID(unsigned int _UID);
        unsigned int GetUID() const;

        void SetAppearCounter(unsigned int _AppearCounter);
        unsigned int GetAppearCounter() const;

        void SetIsFound(bool _Flag);
        bool IsFound() const;

    public:

        CARTrackedObjectPluginFacet();
        ~CARTrackedObjectPluginFacet();

    private:

        bool         m_IsFound;
        unsigned int m_UID;
        unsigned int m_AppearCounter;
    };
} // namespace Dt