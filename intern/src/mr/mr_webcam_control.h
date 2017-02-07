
#pragma once

#include "mr/mr_control.h"

#include <string>

namespace MR
{
    class CWebcamControl : public CControl
    {
    public:

        CWebcamControl();

        ~CWebcamControl();

    public:

        void SetConfiguration(const std::string& _rConfiguration);
        const std::string& GetConfiguration() const;

    private:

        std::string m_Configuration;
        void*       m_OriginalColorFrameBGR;
        void*       m_OriginalColorFrameRGB;
        void*       m_ConvertedColorFrame;

    private:

        virtual void InternStart(const Base::Char* _pCameraParameterFile);

        virtual void InternStop();

        virtual void InternUpdate();

    private:

        void ConvertOriginalToOutput();
    };
} // namespace MR