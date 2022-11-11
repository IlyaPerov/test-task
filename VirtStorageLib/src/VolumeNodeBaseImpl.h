#pragma once

#include "VolumeNode.h"
#include "intfs/NodeEvents.h"
#include "utils/Noncopyable.h"


//
// VolumeNodeBaseImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeBaseImpl :
	public IVolumeNode<KeyT, ValueHolderT>,
	public INodeEventsSubscription<IVolumeNode<KeyT, ValueHolderT>>,
	private NonCopyable
{
protected:
	VolumeNodeBaseImpl() = default;
private:
	VolumeNodeBaseImpl(const VolumeNodeBaseImpl&) = delete;
	VolumeNodeBaseImpl(VolumeNodeBaseImpl&&) = delete;

};