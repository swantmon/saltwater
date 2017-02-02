
#pragma once

#include "mr/mr_control.h"

#include "base/base_matrix4x4.h"
#include "base/base_console.h"

#include <cassert>

namespace MR
{
    class CRealSenseControl
    {
    public:

        CRealSenseControl();
        ~CRealSenseControl();

    public:

        void Start();
        void Stop();

        void GetWidth();
        void GetHeight();
        void GetPixelCount();

        template<typename T>
        bool GetDepthBuffer(T* pBuffer)
        {
            static_assert(std::is_arithmetic<T>::value, "T is not arithmetic");

            return false;
        }

        static const int DepthImageWidth;
        static const int DepthImageHeight;
        static const int DepthImagePixelsCount;

	private:

		
    };
} // namespace MR