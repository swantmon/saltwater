
#include "base/base_precompiled.h"

#include "base/base_program_parameters.h"

#include "json.hpp"
using json = nlohmann::json;

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
            BASE_CONSOLE_WARNINGV("Config file %s does not exists!", _rFile.c_str());
        }

        std::ifstream JSONFile("editor.json");

        if (JSONFile.is_open())
        {
            std::string FileContent((std::istreambuf_iterator<char>(JSONFile)), std::istreambuf_iterator<char>());

            InternParseJSONString(FileContent);

            JSONFile.close();
        }
        else
        {
            BASE_CONSOLE_WARNINGV("Config file %s does not exists!", _rFile.c_str());
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

        std::ofstream JSONFile("editor.json");

        if (JSONFile.is_open())
        {
            json FileContent = json::object({});
            
            for (auto& rElement : m_Container)
            {               
                FileContent.push_back({ ConfigStringToJSON(rElement.first), rElement.second });
            }

            /////////////////////////////////////////////////////////////////////////////////////////
            // Now we iterate over the deques and add names as json objects and values at the end
            /////////////////////////////////////////////////////////////////////////////////////////
            
            JSONFile.clear();
            
            JSONFile << FileContent.unflatten().dump(4);

            JSONFile.close();
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

    int CProgramParameters::GetInt(const std::string& _rOption, int _Default)
    {
#ifdef __ANDROID__ 
        if (HasParameter(_rOption)) return atoi(m_Container[_rOption].c_str());
#else
        if (HasParameter(_rOption)) return std::stoi(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    unsigned int CProgramParameters::GetUInt(const std::string& _rOption, unsigned int _Default)
    {
#ifdef __ANDROID__
        if (HasParameter(_rOption)) return atoi(m_Container[_rOption].c_str());
#else
        if (HasParameter(_rOption)) return std::stoul(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    long CProgramParameters::GetLong(const std::string& _rOption, long _Default)
    {
#ifdef __ANDROID__
        char* pEnd;
        if (HasParameter(_rOption)) return strtol(m_Container[_rOption].c_str(), &pEnd, 10);
#else
        if (HasParameter(_rOption)) return std::stol(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    unsigned long CProgramParameters::GetULong(const std::string& _rOption, unsigned long _Default)
    {
#ifdef __ANDROID__
        char* pEnd;
        if (HasParameter(_rOption)) return strtoul(m_Container[_rOption].c_str(), &pEnd, 10);
#else
        if (HasParameter(_rOption)) return std::stoul(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    long long CProgramParameters::GetLongLong(const std::string& _rOption, long long _Default)
    {
#ifdef __ANDROID__
        char* pEnd;
        if (HasParameter(_rOption)) return strtoll(m_Container[_rOption].c_str(), &pEnd, 10);
#else
        if (HasParameter(_rOption)) return std::stoll(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    unsigned long long CProgramParameters::GetULongLong(const std::string& _rOption, unsigned long long _Default)
    {
#ifdef __ANDROID__
        char* pEnd;
        if (HasParameter(_rOption)) return strtoull(m_Container[_rOption].c_str(), &pEnd, 10);
#else
        if (HasParameter(_rOption)) return std::stoull(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::GetBoolean(const std::string& _rOption, bool _Default)
    {
        if (HasParameter(_rOption)) return m_Container[_rOption] == "1" ? true : false;

        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    float CProgramParameters::GetFloat(const std::string& _rOption, float _Default)
    {
#ifdef __ANDROID__ 
        if (HasParameter(_rOption)) return atof(m_Container[_rOption].c_str());
#else
        if (HasParameter(_rOption)) return std::stof(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    double CProgramParameters::GetDouble(const std::string& _rOption, double _Default)
    {
#ifdef __ANDROID__
        char* pEnd;
        if (HasParameter(_rOption)) return strtod(m_Container[_rOption].c_str(), &pEnd);
#else
        if (HasParameter(_rOption)) return std::stod(m_Container[_rOption]);
#endif
        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    const char* CProgramParameters::GetString(const std::string& _rOption, const char* _Default)
    {
        if (HasParameter(_rOption)) return m_Container[_rOption].c_str();

        AddParameter(_rOption, _Default);

        return _Default;
    }

    // -----------------------------------------------------------------------------

    const std::string& CProgramParameters::GetStdString(const std::string& _rOption, const std::string& _rDefault)
    {
        if (HasParameter(_rOption)) return m_Container[_rOption];

        AddParameter(_rOption, _rDefault);

        return _rDefault;
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

    // -----------------------------------------------------------------------------

    void CProgramParameters::InternParseJSONString(const std::string& _rString)
    {
        json FileContent = json::parse(_rString);

        FileContent = FileContent.flatten();

        for (auto Iterator = FileContent.begin(); Iterator != FileContent.end(); ++ Iterator)
        {
            std::string Option = JSONStringToConfig(Iterator.key());
            std::string Value = Iterator.value();
            AddParameter(Option, Value);
        }
    }

    // -----------------------------------------------------------------------------

    std::string CProgramParameters::ConfigStringToJSON(const std::string& _rString)
    {
        std::string String = _rString;
        std::replace(String.begin(), String.end(), ':', '/');
        return '/' + String;
    }

    // -----------------------------------------------------------------------------

    std::string CProgramParameters::JSONStringToConfig(const std::string& _rString)
    {
        std::string String = _rString;
        String.erase(String.begin());
        std::replace(String.begin(), String.end(), '/', ':');
        return String;
    }

} // namespace IO