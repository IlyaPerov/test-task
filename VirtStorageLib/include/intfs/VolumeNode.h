#pragma once

#include "Types.h"
#include "Node.h"
#include "NodeLifespan.h"
#include "NodeContainer.h"

namespace vs
{

template<typename KeyT, typename ValueHolderT>
struct IVolumeNode :
	INode<KeyT, ValueHolderT>,
	INodeContainer<IVolumeNode<KeyT, ValueHolderT>>,
	INodeLifespan
{
	using INodeContainer<IVolumeNode<KeyT, ValueHolderT>>::NodePtr;

	virtual ~IVolumeNode() = default;

	virtual Priority GetPriority() const = 0;
};

} //namespace vs