
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

    public:

        bool HasParameter(const std::string& _rOption);

    private:

        json m_Container;

    private:

        CProgramParameters();
        ~CProgramParameters();
    };
} // namespace IO

namespace IO
{
    template<typename T>
    void CProgramParameters::Add(const std::string& _rOption, const T _Parameter)
    {
        BASE_CONSOLE_INFO((std::string("Creating new config parameter ") + _rOption).c_str());

        std::string Copy = _rOption;

        std::replace(Copy.begin(), Copy.end(), ':', '/');

        m_Container[json::json_pointer("/" + Copy)] = _Parameter;
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    T CProgramParameters::Get(const std::string& _rOption, const T _Default)
    {
        if (HasParameter(_rOption) == false)
        {
            Add(_rOption, _Default);

            return _Default;
        }

        std::string Copy = _rOption;

        std::replace(Copy.begin(), Copy.end(), ':', '/');

        return m_Container[json::json_pointer("/" + Copy)];
    }
} // namespace IO