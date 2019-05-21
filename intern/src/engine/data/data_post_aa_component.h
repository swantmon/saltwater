
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CPostAAComponent : public CComponent<CPostAAComponent>
    {
    public:

        enum EType
        {
            SMAA,
            FXAA,
            NumberOfTypes,
            Undefined = -1,
        };

        void SetType(EType _Type);
        EType GetType();

        void UpdateEffect();

    public:

        CPostAAComponent();
        ~CPostAAComponent();

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
            return new CPostAAComponent();
        }

    private:

        EType m_Type;
    };
} // namespace Dt