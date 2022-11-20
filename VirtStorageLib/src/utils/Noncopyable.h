#pragma once

namespace vs
{

namespace utils
{

class NonCopyable
{
protected:
	NonCopyable() = default;

private:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = delete;
};

} //namespace utils

} //namespace vs