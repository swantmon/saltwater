
#pragma once

#include "base/base_input_event.h"
#include "base/base_vector2.h"

#include "camera/cam_control.h"

namespace Cam
{
    class CARControl : public CControl
    {
    public:
        
        CARControl();
        ~CARControl();
        
    public:
        
        void SetProjectionMatrix(const Base::Float3x3& _rProjectionMatrix);
        void SetMarkerTransformation(const Base::Float3x3& _rRotationMatrix, const Base::Float3& _rTranslation);
        
    private:
        
        virtual void InternOnEvent(const Base::CInputEvent& _rEvent);
        
    private:
        
        virtual void InternUpdate();
    };
}