
#pragma once

#include "json.hpp"

#include "fwd.hpp"

namespace Base
{
    template<typename T>
    inline T ParseString(const std::string& _rString);

    template<>
    inline float ParseString<float>(const std::string& _rString)
    {
        return std::stof(_rString);
    }

    template<>
    inline double ParseString<double>(const std::string& _rString)
    {
        return std::stod(_rString);
    }

    template<>
    inline int ParseString<int>(const std::string& _rString)
    {
        return std::stoi(_rString);
    }

    template<>
    inline unsigned int ParseString<unsigned int>(const std::string& _rString)
    {
        return static_cast<unsigned int>(std::stoul(_rString));
    }

    template<typename T>
    inline std::vector<T> Split(const std::string& _rString, char _Delimiter)
    {
        std::stringstream Stream(_rString);
        std::string Value;
        std::vector<T> Values;

        while (std::getline(Stream, Value, _Delimiter)) {
            Values.push_back(ParseString<T>(Value));
        }

        return Values;
    }

    template<int N, typename T>
    inline void InternToJson(nlohmann::json& j, T _rValue)
    {
        std::stringstream Stream;

        const auto* pData = glm::value_ptr(_rValue);

        for (int i = 0; i < N - 1; ++i)
        {
            Stream << pData[i] << ", ";
        }
        Stream << pData[N - 1];
        j = Stream.str();
    }
}

namespace glm
{
    template<typename T>
    inline void to_json(nlohmann::json& j, const T& _rValue)
    {
        Base::InternToJson<sizeof(T) / sizeof(T::value_type)>(j, _rValue);
    }

    template<typename T>
    inline void from_json(const nlohmann::json& j, T& _rValue)
    {
        BASE_UNUSED(j);
        BASE_UNUSED(_rValue);
        //const auto Values = Base::Split<decltype(_rValue.x)>(j, ',');
    }
}