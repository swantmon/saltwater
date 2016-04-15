
#pragma once

namespace Edit
{
namespace GUI
{
    void Create(int& _rArgc, char** _ppArgv);
    void Destroy();

    void Setup(int _Width, int _Height);

    void Show();

    void ProcessEvents();

    void CreateContext();
    void SwapWindow();

} // namespace GUI
} // namespace Edit
