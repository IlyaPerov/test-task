#pragma once

#include "Node.h"
#include "NodeContainer.h"
#include "VolumeNode.h"
#include "NodeMounter.h"

namespace vs
{

template<typename KeyT, typename ValueHolderT>
struct IVirtualNode :
	INode<KeyT, ValueHolderT>,
	INodeContainer<IVirtualNode<KeyT, ValueHolderT>>,
	INodeMounter<IVolumeNode<KeyT, ValueHolderT>>
{
	virtual ~IVirtualNode() = default;
};

}  //namespace vs