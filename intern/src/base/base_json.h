
#pragma once

#include "json.hpp"

#include <glm.hpp>

inline void to_json(nlohmann::json& j, const glm::vec4& Test);
inline void from_json(const nlohmann::json& j, glm::vec4& Test);