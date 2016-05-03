
#include "test_precompiled.h"

#include "base/base_test_defines.h"


class CTestClass
{
public:
    
    void NoReturnFunction(unsigned int& _rReference)
    {
        if (_rReference == 10)
        {
            _rReference += 1;
        }
    }
    
    void ReturnNullFunction(unsigned int& _rReference)
    {
        if (_rReference != 10) return;
        
        _rReference += 1;
    }
};


BASE_TEST(CallWithDifferentReturns)
{
    CTestClass   TestClass;
    unsigned int NumberOfCalls = 2048;
    
    BASE_TIME_RESET();
    
    for (unsigned int i = 0; i < NumberOfCalls; ++i )
    {
        TestClass.NoReturnFunction(i);
    }
    
    BASE_TIME_LOG(NoReturnFunction);
    
    BASE_TIME_RESET();
    
    for (unsigned int i = 0; i < NumberOfCalls; ++i )
    {
        TestClass.ReturnNullFunction(i);
    }
    
    BASE_TIME_LOG(ReturnNullFunction);
}