
#pragma once

namespace IO
{
    int GetOption(int _Argc, char* _pArgv[], char* _pOptions);

    const char* GetArgument();

    int GetOptionIndex();

    int GetOptionError();

    int GetOptionName();
} // namespace IO
