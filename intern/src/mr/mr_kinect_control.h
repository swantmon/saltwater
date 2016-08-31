
#pragma once

#include "mr/mr_control.h"

namespace MR
{
    class CKinectControl : public CControl
    {
    public:

        CKinectControl();
        ~CKinectControl();

    public:

        void* GetOriginalDepthFrame() const;
        void* GetConvertedDepthFrame() const;

    private:

        virtual void InternStart(const Base::Char* _pCameraParameterFile);
        virtual void InternStop();

        virtual void InternUpdate();

        void ProcessEnvironmentApproximation();
    };
} // namespace MR