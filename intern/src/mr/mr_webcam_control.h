
#pragma once

#include "base/base_input_event.h"
#include "base/base_vector2.h"

#include "mr/mr_control.h"

namespace MR
{
    class CWebcamControl : public CControl
    {
    public:

        CWebcamControl();

        ~CWebcamControl();

    public:

        

    private:

        void*             m_OriginalColorFrame;
        void*             m_OriginalColorFrameRGB;
        void*             m_ConvertedColorFrame;
        Base::Int2        m_OriginalSize;
        Base::Int2        m_ConvertedSize;
        Dt::CTextureCube* m_pCubemap;
        Dt::CTexture2D*   m_pOriginalFrame;
        Dt::CTexture2D*   m_pConvertedFrame;
        SDeviceParameter  m_CameraParameters;
        Base::Float3x3    m_ProjectionMatrix;

    private:

        virtual void InternStart(const SControlDescription& _rDescriptor);

        virtual void InternStop();

        virtual void InternUpdate();

        virtual Dt::CTexture2D* InternGetOriginalFrame();

        virtual Dt::CTexture2D* InternGetConvertedFrame();

        virtual Dt::CTextureCube* InternGetCubemap();

        virtual SDeviceParameter& InternGetCameraParameters();

        virtual Base::Float3x3& InternGetProjectionMatrix();

    private:

        void ConvertOriginalToOutput();
        void ProcessEnvironmentApproximation();
    };
} // namespace MR