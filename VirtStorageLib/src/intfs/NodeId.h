#pragma once

#include "Types.h"

namespace vs
{

namespace internal
{

struct INodeId
{
	virtual ~INodeId() = default;

	virtual NodeId GetId() const noexcept = 0;
};

} //namespace internal

} //namespace vs