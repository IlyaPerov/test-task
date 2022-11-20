#pragma once

#include <type_traits>

#include "utils/UniqueIdGenerator.h"

#include "intfs/NodeId.h"

namespace vs
{

namespace internal
{

//
// NodeIdImpl
//

template<typename BaseT>
class NodeIdImpl:
	public BaseT
{

public:
	
	// INodeId
	NodeId GetId() const noexcept override final
	{
		return m_id;
	}

protected:
	NodeIdImpl() : m_id{utils::UniqueIdGenerator::GetNextUniqueId()}
	{
		static_assert(std::is_base_of_v<INodeId, BaseT>, "BaseT parameter must derive from INodeId");
	}

private:
	const NodeId m_id;
};

} //namespace internal

} //namespace vs
