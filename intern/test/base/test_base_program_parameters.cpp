
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "engine/core/core_program_parameters.h"

using namespace Core;

BASE_TEST(ProgramParametersDefault)
{
    CProgramParameters::GetInstance().Clear();

    // -----------------------------------------------------------------------------

    CProgramParameters::GetInstance().Add("Speed", 4.0f);

    CProgramParameters::GetInstance().Add("Debug", true);

    CProgramParameters::GetInstance().Add("Desc", "It's me Mario");

    CProgramParameters::GetInstance().Add("Instance", 1337);

    CProgramParameters::GetInstance().Add("TimeScale", 2.3);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Left") == true);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Speed") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Debug") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Desc") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Instance") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("TimeScale") == false);

    // -----------------------------------------------------------------------------

    CProgramParameters::GetInstance().Add("Left", std::string("RightKey"));

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Left") == false);

    CProgramParameters::GetInstance().Add("Left", std::string("LeftKey"));

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("Left") == false);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().Get<float>("Speed", 0.0f) == 4.0f);

    BASE_CHECK(CProgramParameters::GetInstance().Get<bool>("Debug", false) == true);

    BASE_CHECK(CProgramParameters::GetInstance().Get<std::string>("Desc", "") == "It's me Mario");

    BASE_CHECK(CProgramParameters::GetInstance().Get<int>("Instance", 0) == 1337);

    BASE_CHECK(CProgramParameters::GetInstance().Get<double>("TimeScale", 0.0) == 2.3);

    BASE_CHECK(CProgramParameters::GetInstance().Get<std::string>("Left", "") == "LeftKey");
}

BASE_TEST(ProgramParametersParseArguments)
{
    CProgramParameters::GetInstance().Clear();

    // -----------------------------------------------------------------------------

    const char* pParameters = R"({ "app_activate_logic": true, "con_color": "255 180 30", "window_name": "Saltwater is cool", "gfx_timeout": 35, "gfx_use_picking": false })";

    CProgramParameters::GetInstance().ParseJSON(pParameters);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("app_activate_logic") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("con_color") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("window_name") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("gfx_timeout") == false);

    BASE_CHECK(CProgramParameters::GetInstance().IsNull("gfx_use_picking") == false);

    // -----------------------------------------------------------------------------

    BASE_CHECK(CProgramParameters::GetInstance().Get<bool>("app_activate_logic", false) == true);

    BASE_CHECK(CProgramParameters::GetInstance().Get<std::string>("con_color", "") == "255 180 30");

    BASE_CHECK(CProgramParameters::GetInstance().Get<std::string>("window_name", "") == "Saltwater is cool");

    BASE_CHECK(CProgramParameters::GetInstance().Get<int>("gfx_timeout", 0) == 35);

    BASE_CHECK(CProgramParameters::GetInstance().Get<float>("gfx_timeout", 0.0f) == 35.0f);

    BASE_CHECK(CProgramParameters::GetInstance().Get<bool>("gfx_use_picking", false) == false);
}

BASE_TEST(ProgramParametersArrays)
{
    CProgramParameters::GetInstance().Clear();

    // -----------------------------------------------------------------------------

    std::vector<float> Floats;

    for (int i = 0; i < 20; ++i)
    {
        Floats.push_back((float)(std::rand() % RAND_MAX));
    }

    CProgramParameters::GetInstance().Add("My Floats", Floats);

    std::vector<float> ResultFloats;

    ResultFloats = CProgramParameters::GetInstance().Get<std::vector<float>>("My Floats", { 1.0f, 0.0f, 5.0f, 0.2f });

    BASE_CHECK(Floats == ResultFloats);
}