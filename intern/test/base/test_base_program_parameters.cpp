
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_program_parameters.h"

using namespace Base;

BASE_TEST(ProgramParametersDefault)
{
    CProgramParameters::GetInstance().AddParameter("Speed", 4.0f);

    CProgramParameters::GetInstance().AddParameter("Debug", true);

    CProgramParameters::GetInstance().AddParameter("Desc", "It's me Mario");

    CProgramParameters::GetInstance().AddParameter("Instance", 1337);

    CProgramParameters::GetInstance().AddParameter("TimeScale", 2.3);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Left") == false);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Speed") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Debug") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Desc") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Instance") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("TimeScale") == true);

    // -----------------------------------------------------------------------------

    CProgramParameters::GetInstance().AddParameter("Left", std::string("RightKey"));

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Left") == true);

    CProgramParameters::GetInstance().AddParameter("Left", std::string("LeftKey"));

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("Left") == true);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetFloat("Speed") == 4.0f);

    BASE_CHECK(CProgramParameters::GetInstance().GetBoolean("Debug") == true);

    BASE_CHECK(CProgramParameters::GetInstance().GetStdString("Desc") == "It's me Mario");

    BASE_CHECK(CProgramParameters::GetInstance().GetInt("Instance") == 1337);

    BASE_CHECK(CProgramParameters::GetInstance().GetDouble("TimeScale") == 2.3);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetStdString("Left") == "RightKey");

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetFloat("Instance") == 1337.0f);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetInt("Speed") == 4);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetBoolean("Desc") == false);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetInt("Debug") == 1);

    // -----------------------------------------------------------------------------

    int NotExisting = 0;

    try
    {
        NotExisting = CProgramParameters::GetInstance().GetInt("NotExisting");
    }
    catch (...)
    {
        BASE_CHECK(NotExisting == 0);
    }

    // -----------------------------------------------------------------------------

    int WrongParameter = 0;

    try
    {
        WrongParameter = CProgramParameters::GetInstance().GetInt("Desc");
    }
    catch (...)
    {
        BASE_CHECK(WrongParameter == 0);
    }
}

BASE_TEST(ProgramParametersArgv)
{
}

BASE_TEST(ProgramParametersFile)
{
}