#include "UniqueIdGenerator.h"

namespace vs
{

namespace utils
{

std::atomic<NodeId> UniqueIdGenerator::g_nextUniqueId = 0;

NodeId UniqueIdGenerator::GetNextUniqueId()
{
	return g_nextUniqueId.fetch_add(1, std::memory_order_relaxed);
}

} //namespace utils

} //namespace vs