
#pragma once

#include "json.hpp"

#include "base_include_glm.h"

#include <string>

namespace Base
{
    ////////////////////////////////////////////////////////////////////////////////////////////
    // Converting between strings and integrals
    ////////////////////////////////////////////////////////////////////////////////////////////

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
        assert(_rString.length() > 0);

        std::string String = _rString;
        
        std::stringstream Stream(String);
        std::string Value;
        std::vector<T> Values;

        while (std::getline(Stream, Value, _Delimiter))
        {
            Values.push_back(ParseString<T>(Value));
        }

        return Values;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Converting between json and glm types
    ////////////////////////////////////////////////////////////////////////////////////////////

    template<int N, typename T>
    inline void InternToJson(nlohmann::json& j, const T& _rValue)
    {
        // operator[] returns a vector from matrices and integrals from vectors
        // That way we can distinguish between them because we want to have individual matrix columns

        const bool IsVector = sizeof(_rValue[0]) == sizeof(T::value_type);

        if (IsVector)
        {
            const auto* pData = glm::value_ptr(_rValue);

            std::stringstream Stream;

            for (int i = 0; i < N - 1; ++i)
            {
                Stream << pData[i] << ", ";
            }
            Stream << pData[N - 1];

            j = Stream.str();
        }
        else
        {
            const auto TransposedMatrix = glm::transpose(_rValue);

            const auto* pData = glm::value_ptr(TransposedMatrix);

            j = nlohmann::json::array();

            const int Width = sizeof(_rValue[0]) / sizeof(T::value_type);

            for (int Column = 0; Column < Width; ++ Column)
            {
                std::stringstream Stream;

                for (int Row = 0; Row < Width - 1; ++Row)
                {
                    Stream << pData[Column * Width + Row] << ", ";
                }
                Stream << pData[Column * Width + (Width - 1)];

                j.push_back(Stream.str());
            }
        }
    }

    template<int N, typename T>
    inline void InternFromJson(const nlohmann::json& j, T& _rValue)
    {
        // operator[] returns a vector from matrices and integrals from vectors
        // That way we can distinguish between them because we want to have individual matrix columns

        const bool IsVector = sizeof(_rValue[0]) == sizeof(T::value_type);
        
        auto* pData = glm::value_ptr(_rValue);

        if (IsVector)
        {
            const auto Values = Base::Split<T::value_type>(j, ',');
            
            for (int i = 0; i < N; ++i)
            {
                pData[i] = Values[i];
            }
        }
        else
        {
            const int Width = sizeof(_rValue[0]) / sizeof(T::value_type);

            for (int Column = 0; Column < Width; ++Column)
            {
                const auto Values = Base::Split<T::value_type>(j[Column], ',');

                for (int i = 0; i < Width; ++i)
                {
                    pData[Column * Width + i] = Values[i];
                }
            }

            _rValue = glm::transpose(_rValue);
        }
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
        Base::InternFromJson<sizeof(T) / sizeof(T::value_type)>(j, _rValue);
    }
}