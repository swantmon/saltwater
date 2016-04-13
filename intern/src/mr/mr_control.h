
#pragma once

#include "base/base_matrix3x3.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "data/data_texture_2d.h"
#include "data/data_texture_cube.h"

#include "mr/mr_camera_parameter.h"
#include "mr/mr_control_description.h"

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

        void Start(const SControlDescription& _rDescriptor);
        void Stop();

        void Update();

        bool IsStarted() const;

    public:

        EType GetType() const;

        Dt::CTexture2D* GetOriginalFrame();
        Dt::CTexture2D* GetConvertedFrame();

        Dt::CTextureCube* GetCubemap();

        SDeviceParameter& GetCameraParameters();

        Base::Float3x3& GetProjectionMatrix();

    public:

        

    protected:

        EType m_Type;
        bool  m_IsStarted;

    protected:

        CControl(EType _Type);

    private:

        virtual void InternStart(const SControlDescription& _rDescription) = 0;
        virtual void InternStop() = 0;

        virtual void InternUpdate() = 0;

        virtual Dt::CTexture2D* InternGetOriginalFrame()  = 0;
        virtual Dt::CTexture2D* InternGetConvertedFrame() = 0;

        virtual Dt::CTextureCube* InternGetCubemap() = 0;

        virtual SDeviceParameter& InternGetCameraParameters() = 0;

        virtual Base::Float3x3& InternGetProjectionMatrix() = 0;
    };
} // namespace MR
