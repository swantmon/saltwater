
#pragma once

#include "mr/mr_control.h"

#include "base/base_matrix4x4.h"
#include "base/base_console.h"

#define NOMINMAX
#include <windows.h>
#include <Kinect.h>
#include <NuiKinectFusionApi.h>

#include <cassert>
#include <mutex>

namespace MR
{
    class CKinectControl
    {
    public:

        CKinectControl();
        ~CKinectControl();

    public:

        void Start();
        void Stop();

        template<typename T>
        bool GetDepthBuffer(T* pBuffer)
        {
            static_assert(std::is_arithmetic<T>::value, "T is not arithmetic");

            IDepthFrame* pDepthFrame = nullptr;
            unsigned int BufferSize;
            unsigned short* pShortBuffer;

            if (m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame) != S_OK)
            {
                return false;
            }

            if (pDepthFrame->AccessUnderlyingBuffer(&BufferSize, &pShortBuffer) != S_OK)
            {
                BASE_CONSOLE_ERROR("Failed to access underlying buffer");
                return false;
            }

            for (int i = 0; i < DepthImagePixelsCount; ++i)
            {
                pBuffer[i] = static_cast<T>(pShortBuffer[i]);
            }

            if (pDepthFrame != nullptr)
            {
                pDepthFrame->Release();
            }

            return true;
        }

        static const int DepthImageWidth;
        static const int DepthImageHeight;
        static const int DepthImagePixelsCount;

	private:

		IKinectSensor*            m_pKinect;
		IDepthFrameReader*        m_pDepthFrameReader;

        unsigned short* m_pDepthImagePixelBuffer;
        DepthSpacePoint* m_pDepthDistortionMap;
        unsigned int* m_pDepthDistortionLT;
        
        __int64 m_CoordinateMappingChangedEvent;

        bool m_VolumeExported;
    };
} // namespace MR