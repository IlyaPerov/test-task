#pragma once

namespace vs
{

namespace utils
{

class NonCopyable
{
public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator = (const NonCopyable&) = delete;

protected:
	NonCopyable() = default;

};

} //namespace utils

} //namespace vs