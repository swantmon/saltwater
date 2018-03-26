
#pragma once

#include "mr/mr_session.h"

namespace MR
{
namespace SessionManager
{
    struct SDisplayRotation
    {
        enum EDisplayRotation
        {
            Rotation0 = 0,
            Rotation90 = 1,
            Rotation180 = 2,
            Rotation270 = 3,
        };
    };
} // namespace SessionManager
} // namespace MR

namespace MR
{
namespace SessionManager
{
    void Initialize();

    void Update();

    void SetTexture(int _TextureId);

    void OnDisplayGeometryChanged(SDisplayRotation::EDisplayRotation _DisplayRotation, int _Width, int _Height);

    void OnPause();

    void OnResume();

    const CSession& GetSession();
} // namespace SessionManager
} // namespace MR
