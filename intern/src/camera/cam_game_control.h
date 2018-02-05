
#pragma once

#include "base/base_input_event.h"

#include "camera/cam_control.h"

namespace Cam
{
    class CGameControl : public CControl
    {
    public:
        
        CGameControl();
        ~CGameControl();
        
    public:
        
        void SetEntity(Dt::CEntity& _rEntity);
        Dt::CEntity* GetEntity();
        const Dt::CEntity* GetEntity() const;

    private:

        Dt::CEntity* m_pMainCameraEntity;
        
    private:
        
        virtual void InternOnEvent(const Base::CInputEvent& _rEvent);
        virtual void InternOnDirtyEntity(Dt::CEntity* _pEntity);
        
    private:
        
        virtual void InternUpdate();
    };
}