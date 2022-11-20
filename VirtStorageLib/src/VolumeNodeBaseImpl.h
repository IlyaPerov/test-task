#pragma once

#include "VolumeNode.h"

#include "intfs/NodeEvents.h"
#include "utils/Noncopyable.h"
#include "intfs/NodeId.h"

namespace vs
{

namespace internal
{

//
// VolumeNodeBaseImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeBaseImpl :
	public IVolumeNode<KeyT, ValueHolderT>,
	public INodeEventsSubscription<IVolumeNode<KeyT, ValueHolderT>>,
	public INodeId,
	private utils::NonCopyable
{
protected:
	VolumeNodeBaseImpl() = default;
private:
	VolumeNodeBaseImpl(const VolumeNodeBaseImpl&) = delete;
	VolumeNodeBaseImpl(VolumeNodeBaseImpl&&) = delete;

};

} //namespace internal

} //namespace vs