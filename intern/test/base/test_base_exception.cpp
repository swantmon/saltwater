
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_exception.h"

using namespace Base;

BASE_TEST(TestBaseException)
{
    try
    {
        int a = 4;
        
        BASE_THROWCV(2, "Die Zahl ist %i und %i!", a, a*a);
    }
    catch (const Base::CException& _rException)
    {
        BASE_CHECK(_rException.GetLine() == 14);
        BASE_CHECK(_rException.GetCode() == 2);
        BASE_CHECK(strcmp(_rException.GetText(), "Die Zahl ist 4 und 16!") == 0);
    }
}