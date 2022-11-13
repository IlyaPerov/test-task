#include "UniqueIdGenerator.h"

std::atomic<NodeId> UniqueIdGenerator::g_nextUniqueId = 0;

NodeId UniqueIdGenerator::GetNextUniquId()
{
	return g_nextUniqueId.fetch_add(1);
}