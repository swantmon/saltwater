
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_program_parameters.h"

#include "editor/edit_application.h"

int main(int _Argc, char* _pArgv[])
{
    Base::CProgramParameters::GetInstance().ParseFile("editor.config");

    try
    {
        Edit::Application::OnStart(_Argc, _pArgv);

        Edit::Application::OnRun();
    }
    catch (const Base::CException& _rException)
    {
        BASE_CONSOLE_ERROR("An Exception stops application");
        BASE_CONSOLE_INFOV(" > Reason:   %s"   , _rException.GetText());
        BASE_CONSOLE_INFOV(" > Code:     %i"   , _rException.GetCode());
        BASE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        BASE_CONSOLE_ERROR("An undefined exception stops application");
    }

    try
    {
        Edit::Application::OnExit();
    }
    catch (const Base::CException& _rException)
    {
        BASE_CONSOLE_ERROR("An Exception stops application");
        BASE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        BASE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        BASE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        BASE_CONSOLE_ERROR("An undefined exception stops application");
    }

    Base::CProgramParameters::GetInstance().WriteFile("editor.config");

    return 0;
}