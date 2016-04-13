
#pragma once

#include "base/base_input_event.h"
#include "base/base_vector2.h"

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

        Base::Int2        m_OriginalSize;
        Base::Int2        m_ConvertedSize;
        Dt::CTexture2D*   m_pOriginalFrame;
        Dt::CTexture2D*   m_pConvertedFrame;
        Dt::CTextureCube* m_pCubemap;
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
    };
} // namespace MR