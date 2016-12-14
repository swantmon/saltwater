
#pragma once

#include "base/base_matrix3x3.h"

#include "data/data_texture_2d.h"

namespace MR
{
    struct SDeviceParameter
    {
        static const unsigned int s_MaximumNumberOfDistortionFactorValues = 9;

        int      m_FrameWidth;
        int      m_FrameHeight;
        double   m_ProjectionMatrix[3][4];
        double   m_DistortionFactor[s_MaximumNumberOfDistortionFactorValues];
        int      m_DistortionFunctionVersion;
        int      m_PixelFormat;
    };
} // namespace MR

namespace MR
{
    class CControl
    {
    public:

        enum EType
        {
            Kinect,
            Webcam,
            NumberOfControls,
            Undefined = -1,
        };

    public:

        virtual ~CControl();

    public:

        void Start(const Base::Char* m_pCameraParameterFile);
        void Stop();

        void Update();

        bool IsStarted() const;

    public:

        EType GetType() const;

        void SetConvertedFrame(Dt::CTexture2D* _pTexture);
        Dt::CTexture2D* GetConvertedFrame();     

    public:

        Dt::CTexture2D* GetOriginalFrame();
        SDeviceParameter& GetCameraParameters();
        Base::Float3x3& GetProjectionMatrix();

    protected:

        EType             m_Type;
        bool              m_IsStarted;
        Dt::CTexture2D*   m_pOriginalFrame;
        Dt::CTexture2D*   m_pConvertedFrame;
        SDeviceParameter  m_CameraParameters;
        Base::Float3x3    m_ProjectionMatrix;

    protected:

        CControl(EType _Type);

    private:

        virtual void InternStart(const Base::Char* m_pCameraParameterFile) = 0;
        virtual void InternStop() = 0;

        virtual void InternUpdate() = 0;
    };
} // namespace MR
