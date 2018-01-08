
#pragma once

namespace MR
{
namespace ControlManager
{
    struct SConfiguration
    {
        void* m_pEnv;
        void* m_pContext;
    };
} // namespace ControlManager
} // namespace MR

namespace MR
{
namespace ControlManager
{
    void OnStart(const SConfiguration& _rConfiguration);
    void OnExit();
    void Update();
} // namespace ControlManager
} // namespace MR
