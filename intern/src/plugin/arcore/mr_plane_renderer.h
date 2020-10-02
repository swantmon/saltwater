
#pragma once

namespace MR
{
namespace PlaneRenderer
{
    struct SSettings
    {
        bool m_ShowPlanes;
    };
} // namespace PlaneRenderer
} // namespace MR

namespace MR
{
namespace PlaneRenderer
{
    void OnStart();
    void OnExit();
    void Update();

    void OnPause();
    void OnResume();

    void Render();

    const SSettings& GetSettings();
    void SetSettings(const SSettings& _rSettings);
    void ResetSettings();
} // namespace PlaneRenderer
} // namespace MR
