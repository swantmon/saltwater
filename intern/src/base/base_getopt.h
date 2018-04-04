
#pragma once

#include "base/base_export.h"

namespace IO
{
    BASE_API int GetOption(int _Argc, char* _pArgv[], char* _pOptions);

    BASE_API const char* GetArgument();

    BASE_API int GetOptionIndex();

    BASE_API int GetOptionError();

    BASE_API int GetOptionName();
} // namespace IO
