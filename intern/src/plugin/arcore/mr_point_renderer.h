
#pragma once

namespace MR
{
namespace PointRenderer
{
    struct SSettings
    {
        bool m_ShowPoints;
    };
} // namespace PointRenderer
} // namespace MR

namespace MR
{
namespace PointRenderer
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
} // namespace PointRenderer
} // namespace MR
