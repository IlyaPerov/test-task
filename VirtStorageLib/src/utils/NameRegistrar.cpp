#include "NameRegistrar.h"
#include <cassert>

namespace vs
{
namespace utils
{

bool NameRegistrar::TryAddName(const std::string& name)
{
	const auto res = m_names.insert(name);
	return res.second;
}

void NameRegistrar::RemoveName(const std::string& name)
{
	auto erased = m_names.erase(name);
	assert(erased);
}

} //namespace utils
} //namespace vs