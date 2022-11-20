#include "NameRegistrar.h"

namespace vs
{
namespace utils
{

bool NameRegistrar::TryAddName(const std::string& name)
{
	const auto res = m_names.insert(name);
	return res.second;
}

} //namespace utils
} //namespace vs