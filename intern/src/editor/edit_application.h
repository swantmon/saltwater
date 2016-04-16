
#pragma once

namespace Edit
{
namespace Application
{
    void OnStart(int& _rArgc, char** _ppArgv);
    void OnExit();
    void OnRun();

    unsigned int GetEditWindowID();
} // namespace Application
} // namespace Edit