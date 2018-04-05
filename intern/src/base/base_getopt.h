
#pragma once

#include "engine/engine_config.h"

namespace IO
{
    ENGINE_API int GetOption(int _Argc, char* _pArgv[], char* _pOptions);

    ENGINE_API const char* GetArgument();

    ENGINE_API int GetOptionIndex();

    ENGINE_API int GetOptionError();

    ENGINE_API int GetOptionName();
} // namespace IO
