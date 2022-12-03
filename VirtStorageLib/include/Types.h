#pragma once

#include <variant>
#include <vector>
#include <string>

namespace vs
{
using blob = std::vector<uint8_t>;

using ValueVariant = std::variant<int32_t, int64_t, double, std::string, blob>;

using Priority = size_t;
constexpr Priority MINIMAL_PRIORITY {};

using Cookie = uint64_t;
constexpr Cookie INVALID_COOKIE = 0;

using NodeId = uint64_t;

} //namespace vs