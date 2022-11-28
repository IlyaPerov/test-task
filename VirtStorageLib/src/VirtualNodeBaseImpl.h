#pragma once

#include "VirtualNode.h"
#include "NodeIdImpl.h"
#include "utils/NonCopyable.h"

namespace vs
{

namespace internal
{

//
// VirtualNodeBaseImpl
//

template<typename KeyT, typename ValueHolderT>
class VirtualNodeBaseImpl :
	public IVirtualNode<KeyT, ValueHolderT>,
	public INodeId,
	private utils::NonCopyable
{
};

} //namespace internal

} //namespace vs