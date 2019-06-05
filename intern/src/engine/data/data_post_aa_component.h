
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

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            int Type;

            _rCodec >> Type;

            m_Type = (EType)Type;
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            _rCodec << (int)m_Type;
        }

        inline IComponent* Allocate() override
        {
            return new CPostAAComponent();
        }

    private:

        EType m_Type;
    };
} // namespace Dt