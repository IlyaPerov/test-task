#pragma once

#include "Node.h"
#include "NodeContainer.h"
#include "VolumeNode.h"
#include "NodeMounter.h"


template<typename KeyT, typename ValueHolderT>
struct IVirtualNode :
    INode<KeyT, ValueHolderT>,
    INodeContainer<IVirtualNode<KeyT, ValueHolderT>>,
    INodeMounter<IVolumeNode<KeyT, ValueHolderT>>
{
    //using VolumeNodeType = typename IVolumeNode<KeyT, ValueHolderT>;
    //using VolumeNodePtr = typename VolumeNodeType::NodePtr;

    //using ForEachMountedFunctorType = std::function<void(VolumeNodePtr)>;
    //using FindMountedIfFunctorType = std::function<bool(VolumeNodePtr)>;
    //using UnmountIfFunctorType = FindMountedIfFunctorType;

    virtual ~IVirtualNode() = default;

    //virtual void Mount(VolumeNodePtr node) = 0;
    //virtual void Unmount(VolumeNodePtr node) = 0;
    //virtual void ForEachMounted(ForEachMountedFunctorType f) = 0;
    //virtual VolumeNodePtr FindMountedIf(FindMountedIfFunctorType f) = 0;
    //virtual void UnmountIf(UnmountIfFunctorType f) = 0;
};
