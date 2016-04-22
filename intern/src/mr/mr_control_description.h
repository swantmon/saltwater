#pragma once

#include "base/base_vector2.h"
#include "base/base_typedef.h"

namespace Dt
{
    CTexture2D;
    CTextureCube;
} // namespace Dt

namespace MR
{
    struct SControlDescription
    {
        Dt::CTexture2D*   m_pOutputBackground;
        Dt::CTextureCube* m_pOutputCubemap;
        const Base::Char* m_pCameraParameterFile;
        unsigned int      m_DeviceNumber;
    };
} // namespace AR