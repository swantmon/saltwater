#pragma once

#include "base/base_vector2.h"
#include "base/base_typedef.h"

namespace MR
{
    struct SControlDescription
    {
        const Base::Char* m_pCameraParameterFile;
        unsigned int      m_DeviceNumber;
        Base::Int2        m_OutputSize;
    };
} // namespace AR