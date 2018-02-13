
#include "base/base_precompiled.h"

#include "base_json.h"

#include "base/base_include_glm.h"

inline void to_json(nlohmann::json& j, const glm::vec4& Test)
{
    std::stringstream Stream;
    Stream << Test.x << Test.y << Test.z << Test.w;
    j = Stream.str();
}

inline void from_json(const nlohmann::json& j, glm::vec4& Test) {
    std::string String = j;
}