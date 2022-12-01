#pragma once

#include  <exception>
#include  "Types.h"

namespace vs
{

class ActionOnRemovedNodeException :
	public std::exception
{
public:
	explicit ActionOnRemovedNodeException(NodeId nodeId): m_nodeId{nodeId}
	{}

	const char* what() const noexcept override
	{
		return "Cannot perform an action because the target node was removed from hierarchy";
	}

	NodeId TargetNodeId() const noexcept
	{
		return m_nodeId;
	}

private:
	NodeId m_nodeId;

};

} //namespace vs