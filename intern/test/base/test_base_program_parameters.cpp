
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_program_paramaters.h"

using namespace Base;

BASE_TEST(ProgramParameters)
{
    bool HasParameter = CProgramParameters::GetInstance().HasParameter("Test");

    BASE_CHECK(HasParameter == false);
}