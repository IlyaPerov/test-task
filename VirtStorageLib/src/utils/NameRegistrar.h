#pragma once

#include <string>
#include <unordered_set>

namespace vs
{

namespace utils
{

class NameRegistrar
{
public:
	bool TryAddName(const std::string& name);
	void RemoveName(const std::string& name);

private:
	std::unordered_set<std::string> m_names;
};

} //namespace utils

} //namespace vs