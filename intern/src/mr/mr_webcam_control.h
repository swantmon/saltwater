
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

        void SetConfiguration(const Base::Char* _pConfiguration);
        const Base::Char* GetConfiguration() const;

    private:

        const Base::Char* m_pConfiguration;
        void*             m_OriginalColorFrame;
        void*             m_OriginalColorFrameRGB;
        void*             m_ConvertedColorFrame;

    private:

        virtual void InternStart(const Base::Char* _pCameraParameterFile);

        virtual void InternStop();

        virtual void InternUpdate();

    private:

        void ConvertOriginalToOutput();
    };
} // namespace MR