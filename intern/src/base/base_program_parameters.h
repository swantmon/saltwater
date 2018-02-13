
#pragma once

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_include_json.h"

#include <string>

namespace IO
{
    class CProgramParameters
    {
    public:

        static CProgramParameters& GetInstance();

    public:

        void ParseJSON(const std::string& _rJSON);

        void ParseFile(const std::string& _rFile);
        void WriteFile(const std::string& _rFile);

    public:

        void Clear();

    public:

        template<typename T>
        void Add(const std::string& _rOption, const T _rParameter);
        
        template<typename T>
        const T Get(const std::string& _rOption, const T _Default);
        const std::string Get(const std::string& _rOption, const char* _Default);

        bool IsNull(const std::string& _rOption);

    private:

        nlohmann::json m_Container;

    private:

        CProgramParameters();
        ~CProgramParameters();

    private:

        nlohmann::json::json_pointer ConvertOptionToJSONPointer(const std::string& _rOption);
    };
} // namespace IO

namespace IO
{
    template<typename T>
    void CProgramParameters::Add(const std::string& _rOption, const T _Parameter)
    {
        BASE_CONSOLE_INFOV("Creating new config parameter %s", _rOption.c_str());

        m_Container[ConvertOptionToJSONPointer(_rOption)] = _Parameter;
    }
    
    // -----------------------------------------------------------------------------

    template<typename T>
    const T CProgramParameters::Get(const std::string& _rOption, const T _Default)
    {
        try
        {
            if (IsNull(_rOption))
            {
                Add(_rOption, _Default);

                return _Default;
            }

            return m_Container[ConvertOptionToJSONPointer(_rOption)];
        }
        catch (const nlohmann::json::exception& _rException)
        {
            BASE_CONSOLE_ERRORV("Getting value of option \"%s\" from program parameters failed with error: \"%s\"", _rOption.c_str(), _rException.what());
        }

        return _Default;
    }
} // namespace IO