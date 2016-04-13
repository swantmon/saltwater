
#pragma once

#include "base/base_defines.h"

#include <sstream>

namespace UT
{

namespace Suite
{

    void Reset(std::ostream* _pStream = 0);
    
    void RegisterTest(void (*_TestFtr)());
    void RunTests();

    void SetTestSource(const char* _pFileName, const char* _pFunctionName, int _LineNumber);

    void LogFailure();
    void LogException(const char* _pException);
    void LogCondition(const char* _pCondition);
    void LogTime(const char* _pMessage);
    
    void ResetTime();

    std::ostringstream& GetLogStream();

    unsigned int GetTestCount();
    unsigned int GetFailedTestCount();

} // namespace Suite

} // namespace UT