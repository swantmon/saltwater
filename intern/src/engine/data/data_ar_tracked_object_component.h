
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"

namespace Dt
{
    class ENGINE_API CARTrackedObjectPluginComponent : public CComponent<CARTrackedObjectPluginComponent>
    {
    public:

        void SetUID(unsigned int _UID);
        unsigned int GetUID() const;

        void SetAppearCounter(unsigned int _AppearCounter);
        unsigned int GetAppearCounter() const;

        void SetIsFound(bool _Flag);
        bool IsFound() const;

    public:

        CARTrackedObjectPluginComponent();
        ~CARTrackedObjectPluginComponent();

    private:

        bool         m_IsFound;
        unsigned int m_UID;
        unsigned int m_AppearCounter;
    };
} // namespace Dt