#pragma once

#include "base/base_defines.h"

#include <algorithm> 
#include <cctype>
#include <locale>
#include <string>

namespace CORE
{
    static inline void TrimLeft(std::string &_rString)
    {
        _rString.erase(_rString.begin(), std::find_if(_rString.begin(), _rString.end(), [](int ch)
        {
            return !std::isspace(ch);
        }));
    }

    // -----------------------------------------------------------------------------

    static inline void TrimRight(std::string &_rString)
    {
        _rString.erase(std::find_if(_rString.rbegin(), _rString.rend(), [](int ch)
        {
            return !std::isspace(ch);
        }).base(), _rString.end());
    }

    // -----------------------------------------------------------------------------

    static inline void Trim(std::string &_rString)
    {
        TrimLeft(_rString);
        TrimRight(_rString);
    }
} // namespace CORE