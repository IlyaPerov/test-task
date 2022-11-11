#pragma once

#include <variant>
#include <string>



using ValueVariant = std::variant<int32_t, int64_t, double, std::string>;
using Priority = size_t;
using Cookie = int64_t;