
#pragma once

#include "base/base_console.h"

#include "json.hpp"
using nlohmann::json;

#include <string>

namespace IO
{
    class CProgramParameters
    {
    public:

        static CProgramParameters& GetInstance();

    public:

        void ParseFile(const std::string& _rFile);
        void WriteFile(const std::string& _rFile);

    public:

        template<typename T>
        void Add(const std::string& _rOption, const T _rParameter);

        template<typename T>
        T Get(const std::string& _rOption, const T _Default);

        bool IsNull(const std::string& _rOption);

    private:

        json m_Container;

    private:

        CProgramParameters();
        ~CProgramParameters();

    private:

        json::json_pointer ConvertOptionToJSONPointer(const std::string& _rOption);
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
    T CProgramParameters::Get(const std::string& _rOption, const T _Default)
    {
        if (IsNull(_rOption))
        {
            Add(_rOption, _Default);

            return _Default;
        }

        return m_Container[ConvertOptionToJSONPointer(_rOption)];
    }
} // namespace IO