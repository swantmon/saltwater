
#pragma once

#include "mr/mr_control.h"

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

        void Update();
    };
} // namespace MR