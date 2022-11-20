#pragma once

#include  <stdexcept>

namespace vs
{

class ActionOnRemovedNodeException :
	public std::logic_error
{
public:
	explicit ActionOnRemovedNodeException(const std::string& message) : std::logic_error(message)
	{
		;
	}
};

} //namespace vs