#pragma once

#include "base/base_input_event.h"

#include "data/data_entity.h"

namespace Scpt
{
    class IScript
    {
    public:

        inline Dt::CEntity* GetEntity();
        inline Dt::CTransformationFacet* GetTransformation();

    public:

        virtual void Start() = 0;
        virtual void Exit() = 0;
        virtual void Update() = 0;

        virtual void OnInput(const Base::CInputEvent& _rEvent) = 0;

        virtual ~IScript() {};

    private:

        Dt::CEntity* m_pEntity;
        Base::ID     m_ID;
        bool         m_IsActive;
        bool         m_IsStarted;

    private:

        friend class CScriptManager;
    };
} // namespace Scpt

namespace Scpt
{
    inline Dt::CEntity* IScript::GetEntity()
    {
        return m_pEntity;
    }

    // -----------------------------------------------------------------------------

    inline Dt::CTransformationFacet* IScript::GetTransformation()
    {
        if (m_pEntity == nullptr) return nullptr;

        return m_pEntity->GetTransformationFacet();
    }
} // namespace Scpt