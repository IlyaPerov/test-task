#pragma once

#include "VolumeNode.h"


//
// VolumeNodeBaseImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeBaseImpl :
	public IVolumeNode<KeyT, ValueHolderT>
{
protected:
	VolumeNodeBaseImpl() = default;
private:
	VolumeNodeBaseImpl(const VolumeNodeBaseImpl&) = delete;
	VolumeNodeBaseImpl(VolumeNodeBaseImpl&&) = delete;

};