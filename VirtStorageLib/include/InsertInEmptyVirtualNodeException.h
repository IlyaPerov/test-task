#pragma once

#include  <exception>
#include  "Types.h"

namespace vs
{

class InsertInEmptyVirtualNodeException :
	public std::exception
{
public:
	const char* what() const noexcept override
	{
		return "Cannot perform insertion because there are no nodes mounted the target virtual node";
	}
};

} //namespace vs