#pragma once

#include "VolumeNode.h"

#include "intfs/NodeEvents.h"
#include "intfs/NodeId.h"
#include "utils/NonCopyable.h"

namespace vs
{

namespace internal
{

//
// VolumeNodeBaseImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeBase :
	public IVolumeNode<KeyT, ValueHolderT>,
	public INodeEventsSubscription<IVolumeNode<KeyT, ValueHolderT>>,
	public INodeId,
	utils::NonCopyable
{
};

} //namespace internal

} //namespace vs