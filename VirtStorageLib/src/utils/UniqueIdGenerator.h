#pragma once

#include <atomic>

#include "Types.h"

class UniqueIdGenerator
{
public:

	static NodeId GetNextUniquId();

private:
	static std::atomic<NodeId> g_nextUniqueId;


};