
#pragma once

#include "json.hpp"

#include "fwd.hpp"

namespace glm
{
    void to_json(nlohmann::json& j, const glm::vec4& _rValue);
    void from_json(const nlohmann::json& j, glm::vec4&  _rValue);
}