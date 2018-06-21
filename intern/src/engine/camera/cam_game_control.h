
#pragma once

#include "base/base_input_event.h"

#include "engine/camera/cam_control.h"

namespace Cam
{
    class ENGINE_API CGameControl : public CControl
    {
    public:
        
        CGameControl();
        ~CGameControl();

    private:

        Dt::CEntity* m_pRelatedEntity;
        
    private:
        
        void InternOnEvent(const Base::CInputEvent& _rEvent) override;
        void InternOnDirtyEntity(Dt::CEntity* _pEntity) override;
        void InternOnDirtyComponent(Dt::IComponent* _pComponent) override;
        
    private:
        
        virtual void InternUpdate();

    private:

        void UpdateTransformation(Dt::CEntity* _pEntity);
        void UpdateSettings(Dt::IComponent* _pComponent);
        void LookupNewRelatedEntity();
    };
}