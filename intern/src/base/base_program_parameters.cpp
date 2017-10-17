
#include "base/base_precompiled.h"

#include "base/base_program_parameters.h"

#include <algorithm> 
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

    void CProgramParameters::ParseArguments(const std::string& _rArguments)
    {
        InternParseString(_rArguments, ';');
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::ParseFile(const std::string& _rFile)
    {
        std::ifstream File(_rFile.c_str());

        if (File.is_open())
        {
            std::string FileContent((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());

            InternParseString(FileContent, '\n');

            File.close();
        }
        else
        {
            BASE_CONSOLE_ERRORV("The file %s could not be passed.", _rFile.c_str());
        }
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::WriteFile(const std::string& _rFile)
    {
        std::ofstream File(_rFile.c_str());

        if (File.is_open())
        {
            File.clear();

            for (auto& rElement : m_Container)
            {
                File << rElement.first << " = " << rElement.second << std::endl;
            }

            File.close();
        }
        else
        {
            BASE_CONSOLE_ERRORV("Save file %s could not be opened.", _rFile.c_str());
        }
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::AddParameter(const std::string& _rOption, bool _Bool)
    {
        m_Container.insert(COptionParameterPair(_rOption, _Bool ? "1" : "0"));
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::AddParameter(const std::string& _rOption, const char* _pText)
    {
        m_Container.insert(COptionParameterPair(_rOption, _pText));
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::AddParameter(const std::string& _rOption, const std::string& _rText)
    {
        m_Container.insert(COptionParameterPair(_rOption, _rText));
    }

    // -----------------------------------------------------------------------------

    int CProgramParameters::GetInt(const std::string& _rOption)
    {
        return std::stoi(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    unsigned int CProgramParameters::GetUInt(const std::string& _rOption)
    {
        return std::stoul(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    long CProgramParameters::GetLong(const std::string& _rOption)
    {
        return std::stol(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    unsigned long CProgramParameters::GetULong(const std::string& _rOption)
    {
        return std::stoul(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    long long CProgramParameters::GetLongLong(const std::string& _rOption)
    {
        return std::stoll(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    unsigned long long CProgramParameters::GetULongLong(const std::string& _rOption)
    {
        return std::stoull(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::GetBoolean(const std::string& _rOption)
    {
        return m_Container[_rOption] == "1" ? true : false;
    }

    // -----------------------------------------------------------------------------

    float CProgramParameters::GetFloat(const std::string& _rOption)
    {
        return std::stof(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    double CProgramParameters::GetDouble(const std::string& _rOption)
    {
        return std::stod(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    const char* CProgramParameters::GetString(const std::string& _rOption)
    {
        return m_Container[_rOption].c_str();
    }

    // -----------------------------------------------------------------------------

    const std::string& CProgramParameters::GetStdString(const std::string& _rOption)
    {
        return m_Container[_rOption];
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::HasParameter(const std::string& _rOption)
    {
        return m_Container.find(_rOption) != m_Container.end();
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::InternParseString(const std::string& _rString, const char _Delimiter)
    {
        // -----------------------------------------------------------------------------
        // Trimming
        // Code from: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
        // -----------------------------------------------------------------------------
        auto TrimLeft = [&](std::string& _rString) 
        {
            _rString.erase(_rString.begin(), std::find_if(_rString.begin(), _rString.end(), [](int _Character) 
            {
                return !std::isspace(_Character);
            }));
        };

        auto TrimRight = [&](std::string& _rString) 
        {
            _rString.erase(std::find_if(_rString.rbegin(), _rString.rend(), [](int _Character) 
            {
                return !std::isspace(_Character);
            }).base(), _rString.end());
        };

        auto Trim = [&](std::string& _rString) 
        {
            TrimLeft(_rString);
            TrimRight(_rString);
        };

        // -----------------------------------------------------------------------------
        // Split string into several parameters
        // -----------------------------------------------------------------------------
        std::stringstream StreamOfParameter(_rString);
        std::string Parameter;

        while (std::getline(StreamOfParameter, Parameter, _Delimiter))
        {
            // -----------------------------------------------------------------------------
            // Add parameter to container
            // -----------------------------------------------------------------------------
            size_t PositionOfDelimiter = Parameter.find_first_of('=');

            std::string Option = Parameter.substr(0, PositionOfDelimiter);
            std::string Value  = Parameter.substr(PositionOfDelimiter + 1, Parameter.length());

            Trim(Option);
            Trim(Value);

            AddParameter(Option, Value);
        }
    }
} // namespace IO