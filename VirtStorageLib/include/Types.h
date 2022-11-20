#pragma once

#include <variant>
#include <string>

namespace vs
{

using ValueVariant = std::variant<int32_t, int64_t, double, std::string>;

using Priority = size_t;

using Cookie = uint64_t;
constexpr Cookie INVALID_COOKIE = 0;

using NodeId = uint64_t;

} //namespace vs