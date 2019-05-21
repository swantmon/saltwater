
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

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

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CARTrackedObjectPluginComponent();
        }

    private:

        bool         m_IsFound;
        unsigned int m_UID;
        unsigned int m_AppearCounter;
    };
} // namespace Dt