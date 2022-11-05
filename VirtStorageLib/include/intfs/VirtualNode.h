#pragma once

#include "Node.h"
#include "NodeContainer.h"
#include "VolumeNode.h"


template<typename KeyT, typename ValueHolderT>
struct IVirtualNode :
    INode<KeyT, ValueHolderT>,
    INodeContainer<IVirtualNode<KeyT, ValueHolderT>>
{
    using VolumeNodeType = typename IVolumeNode<KeyT, ValueHolderT>;
    using VolumeNodePtr = typename VolumeNodeType::NodePtr;

    virtual ~IVirtualNode() = default;

    virtual void Mount(VolumeNodePtr node) = 0;
    virtual void Unmount(VolumeNodePtr node) = 0;
};
