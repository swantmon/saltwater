
#include "base/base_precompiled.h"

#include "base/base_program_parameters.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <iterator>

namespace IO
{
    CProgramParameters& CProgramParameters::GetInstance()
    {
        static CProgramParameters s_Sinstance;

        return s_Sinstance;
    }
} // namespace IO

namespace IO
{
    CProgramParameters::CProgramParameters()
    {
    }

    // -----------------------------------------------------------------------------

    CProgramParameters::~CProgramParameters()
    {
        m_Container.clear();
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::ParseFile(const std::string& _rFile)
    {
        std::ifstream JSONFile(_rFile.c_str());

        if (JSONFile.is_open())
        {
            m_Container = json::parse(JSONFile, nullptr, false);

            JSONFile.close();
        }
        else
        {
            BASE_CONSOLE_WARNINGV("Config file %s does not exist!", _rFile.c_str());
        }
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::WriteFile(const std::string& _rFile)
    {
        std::ofstream JSONFile(_rFile.c_str());

        if (JSONFile.is_open())
        {
            JSONFile.clear();

            JSONFile << std::setw(4) << m_Container << std::endl;

            JSONFile.close();
        }
        else
        {
            BASE_CONSOLE_ERRORV("Save file %s could not be opened.", _rFile.c_str());
        }
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::IsNull(const std::string& _rOption)
    {
        return m_Container[ConvertOptionToJSONPointer(_rOption)].is_null();
    }

    // -----------------------------------------------------------------------------

    json::json_pointer CProgramParameters::ConvertOptionToJSONPointer(const std::string& _rOption)
    {
        std::string Copy = _rOption;

        std::replace(Copy.begin(), Copy.end(), ':', '/');

        return json::json_pointer("/" + Copy);
    }
} // namespace IO