
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_getopt.h"

#include <string>

using namespace Base;

BASE_TEST(GetOptDefault)
{
    int Argc = 4;
    char* pArgv[] =
    {
        { "/c/users/" },
        { "-h" },
        { "-f editor.config" },
        { "--parameters Rounds=5;StartTime=0;Help=No help here" },
    };

    int MoreArguments;
    const char* pValue;

    for (; (MoreArguments = Base::GetOption(Argc, pArgv, "hf:-:")) != -1; )
    {
        switch (MoreArguments)
        {
        case '?':
            BASE_CHECK(false);
            break;

        case 'h':
            BASE_CHECK(true);
            break;

        case 'f':
            pValue = Base::GetArgument();

            BASE_CHECK(strcmp(pValue, "editor.config"));
            break;

        case '-':
            pValue = Base::GetArgument();

            BASE_CHECK(std::string(pValue) == "parameters Rounds=5;StartTime=0;Help=No help here");
            break;

        default:
            break;
        }
    }
}