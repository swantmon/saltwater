
#include "editor/edit_precompiled.h"

#include "base/base_exception.h"
#include "base/base_getopt.h"

#include "editor/edit_application.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

#include <string>

#include "windows.h"

int main(int _Argc, char* _pArgv[])
{
    const std::string VerbosityNameString = "console:verbose";
    
    std::string ParameterFile = "editor.config";

    int VerbosityLevel = 0;

    int MoreArguments;
    for (; (MoreArguments = Base::GetOption(_Argc, _pArgv, "f:p:v")) != -1; )
    {
        switch (MoreArguments)
        {
        case 'f':
            ParameterFile = Base::GetArgument();
            break;

        case 'v':
            ++ VerbosityLevel;
            break;

        case 'p':
            std::string Argument = Base::GetArgument();

            size_t PositionOfSpace = Argument.find_first_of('=');

            std::string Option = Argument.substr(0, PositionOfSpace);
            std::string Value  = Argument.substr(PositionOfSpace + 1, Argument.length());

            Core::CProgramParameters::GetInstance().Add(Option, Value);
            break;
        }
    }

    if (VerbosityLevel == 0)
    {
        Core::CProgramParameters::GetInstance().ParseFile(ParameterFile);
        VerbosityLevel = Core::CProgramParameters::GetInstance().Get(VerbosityNameString, 3);
    }
    else
    {
        VerbosityLevel = Core::CProgramParameters::GetInstance().Get(VerbosityNameString, VerbosityLevel);
        Core::CProgramParameters::GetInstance().ParseFile(ParameterFile);
    }

    Core::CConsole::GetInstance().SetVerbosityLevel(VerbosityLevel);

    try
    {
        Edit::Application::OnStart(_Argc, _pArgv);

        Edit::Application::OnRun();
    }
    catch (const Base::CException& _rException)
    {
        ENGINE_CONSOLE_ERROR("An Exception stops application");
        ENGINE_CONSOLE_INFOV(" > Reason:   %s"   , _rException.GetText());
        ENGINE_CONSOLE_INFOV(" > Code:     %i"   , _rException.GetCode());
        ENGINE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        ENGINE_CONSOLE_ERROR("An undefined exception stops application");
    }

    try
    {
        Edit::Application::OnExit();
    }
    catch (const Base::CException& _rException)
    {
        ENGINE_CONSOLE_ERROR("An Exception stops application");
        ENGINE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        ENGINE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        ENGINE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        ENGINE_CONSOLE_ERROR("An undefined exception stops application");
    }

    Core::CProgramParameters::GetInstance().WriteFile(ParameterFile);

    return 0;
}

// Make sure that the program is run with the dedicated GPU instead of the integrated one

#ifdef _WIN32

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#endif // _WIN32