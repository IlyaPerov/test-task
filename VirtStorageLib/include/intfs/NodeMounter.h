#pragma once

#include <memory>
#include <functional>

template<typename MountableNodeT>
struct INodeMounter
{
    using MountableNodeType = MountableNodeT;
    using MountableNodePtr = typename MountableNodeT::NodePtr;
    
    using ForEachMountedFunctorType = std::function<void(MountableNodePtr)>;
    using FindMountedIfFunctorType = std::function<bool(MountableNodePtr)>;
    using UnmountIfFunctorType = FindMountedIfFunctorType;

    virtual ~INodeMounter() = default;

    virtual void Mount(MountableNodePtr node) = 0;
    virtual void Unmount(MountableNodePtr node) = 0;
    virtual void ForEachMounted(ForEachMountedFunctorType f) = 0;
    virtual MountableNodePtr FindMountedIf(FindMountedIfFunctorType f) = 0;
    virtual void UnmountIf(UnmountIfFunctorType f) = 0;
};