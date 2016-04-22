
#pragma once

#include "mr/mr_control.h"

namespace MR
{
    class CWebcamControl : public CControl
    {
    public:

        CWebcamControl();

        ~CWebcamControl();

    public:

        void SetDeviceNumber(unsigned int _DeviceNumber);
        unsigned int GetDeviceNumber() const;

    private:

        unsigned int m_DeviceNumber;
        void*        m_OriginalColorFrame;
        void*        m_OriginalColorFrameRGB;
        void*        m_ConvertedColorFrame;

    private:

        virtual void InternStart(const Base::Char* _pCameraParameterFile);

        virtual void InternStop();

        virtual void InternUpdate();

    private:

        void ConvertOriginalToOutput();
        void ProcessEnvironmentApproximation();
    };
} // namespace MR