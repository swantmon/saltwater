
#pragma once

namespace Edit
{
namespace Runtime
{
    void OnStart(int& _rArgc, char** _ppArgv);
    void OnExit();
    void OnRun();

    unsigned int GetEditWindowID();
} // namespace Runtime
} // namespace Edit