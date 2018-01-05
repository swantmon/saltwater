
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_getopt.h"
#include "base/base_program_parameters.h"

#include "editor/edit_application.h"

#include <string>

int main(int _Argc, char* _pArgv[])
{
    const std::string VerbosityNameString = "console_verbose";
    
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

            Base::CProgramParameters::GetInstance().AddParameter(Option, Value);
            break;
        }
    }

    VerbosityLevel = Base::CProgramParameters::GetInstance().GetInt(VerbosityNameString);

    Base::CProgramParameters::GetInstance().AddParameter(VerbosityNameString, VerbosityLevel);

    Base::CProgramParameters::GetInstance().ParseFile(ParameterFile);

    VerbosityLevel = Base::CProgramParameters::GetInstance().GetInt(VerbosityNameString, 3);

    Base::CConsole::GetInstance().SetVerbosityLevel(VerbosityLevel);

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

    Base::CProgramParameters::GetInstance().WriteFile(ParameterFile);

    return 0;
}