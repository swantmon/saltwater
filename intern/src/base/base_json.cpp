
#include "base/base_precompiled.h"

#include "base_json.h"

#include "base/base_include_glm.h"

template<typename T>
T ParseString(const std::string& _rString);

template<>
float ParseString<float>(const std::string& _rString)
{
    return std::stof(_rString);
}

template<>
double ParseString<double>(const std::string& _rString)
{
    return std::stod(_rString);
}

template<>
int ParseString<int>(const std::string& _rString)
{
    return std::stoi(_rString);
}

template<>
unsigned ParseString<unsigned>(const std::string& _rString)
{
    return static_cast<unsigned>(std::stoul(_rString));
}

template<typename T>
std::vector<T> Split(const std::string& _rString, char _Delimiter)
{
    std::stringstream Stream(_rString);
    std::string Value;
    std::vector<T> Values;

    while (std::getline(Stream, Value, _Delimiter)) {
        Values.push_back(ParseString<T>(Value));
    }

    return Values;
}

namespace glm
{
    void to_json(nlohmann::json& j, const glm::vec4& _rValue)
    {
        std::stringstream Stream;
        Stream << _rValue.x << ", " << _rValue.y << ", " << _rValue.z << ", " << _rValue.w;
        j = Stream.str();
    }

    void from_json(const nlohmann::json& j, glm::vec4& _rValue)
    {
        auto Values = Split<float>(j, ',');
    }
}