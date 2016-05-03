//
//  main.cpp
//  test
//
//  Created by Tobias Schwandt on 24/02/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#include "test_precompiled.h"

#include "base/base_test_suite.h"

#include <iostream>

int main(int _Argc, const char* _pArgv[])
{
    BASE_UNUSED(_Argc);
    BASE_UNUSED(_pArgv);

    // -----------------------------------------------------------------------------
    // Prepare tests
    // -----------------------------------------------------------------------------
    ::Base::Suite::Reset(&std::cout);
    
    // -----------------------------------------------------------------------------
    // Run tests
    // -----------------------------------------------------------------------------
    ::Base::Suite::RunTests();
    
    // -----------------------------------------------------------------------------
    // Output of test results
    // -----------------------------------------------------------------------------
    std::cout << ::Base::Suite::GetLogStream().str();
    
    if (::Base::Suite::GetFailedTestCount() > 0)
    {
        return 1;
    }
    
    return 0;
}
