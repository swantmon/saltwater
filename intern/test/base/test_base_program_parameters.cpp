
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

BASE_TEST(ProgramParametersParseString)
{
    const char* pParameters = "";

    CProgramParameters::GetInstance().ParseArguments(pParameters);

    pParameters = "app_activate_logic=1;con_color=255 180 30;window_name=Saltwater is cool;gfx_timeout=35;gfx_use_picking=0";

    CProgramParameters::GetInstance().ParseArguments(pParameters);

    pParameters = "check timing=1";

    CProgramParameters::GetInstance().ParseArguments(pParameters);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("app_activate_logic") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("con_color") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("window_name") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("gfx_timeout") == true);

    BASE_CHECK(CProgramParameters::GetInstance().HasParameter("gfx_use_picking") == true);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().GetBoolean("app_activate_logic") == true);

    BASE_CHECK(CProgramParameters::GetInstance().GetStdString("con_color") == "255 180 30");

    BASE_CHECK(CProgramParameters::GetInstance().GetStdString("window_name") == "Saltwater is cool");

    BASE_CHECK(CProgramParameters::GetInstance().GetInt("gfx_timeout") == 35);

    BASE_CHECK(CProgramParameters::GetInstance().GetFloat("gfx_timeout") == 35.0f);

    BASE_CHECK(CProgramParameters::GetInstance().GetBoolean("gfx_use_picking") == false);

    BASE_CHECK(CProgramParameters::GetInstance().GetBoolean("check timing") == true);
}

BASE_TEST(ProgramParametersParseFile)
{
}