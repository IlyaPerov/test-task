#pragma once

#include <atomic>

#include "Types.h"

namespace vs
{

namespace utils
{

class UniqueIdGenerator
{
public:

	static NodeId GetNextUniqueId();

private:
	static std::atomic<NodeId> g_nextUniqueId;


};

} //namespace utils

} //namespace vs