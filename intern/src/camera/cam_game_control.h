
#pragma once

#include "base/base_input_event.h"
#include "base/base_vector2.h"

#include "camera/cam_control.h"

#include "data/data_entity.h"

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
        
    private:
        
        virtual void InternUpdate();

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
}