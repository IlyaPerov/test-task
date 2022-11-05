#pragma once

#include <memory>
#include "Types.h"
#include "Node.h"
#include "NodeContainer.h"

template<typename KeyT, typename ValueHolderT>
struct IVolumeNode :
    INode<KeyT, ValueHolderT>, 
    INodeContainer<IVolumeNode<KeyT, ValueHolderT>>
{
    using INodeContainer<IVolumeNode<KeyT, ValueHolderT>>::NodePtr;

    virtual ~IVolumeNode() = default;

    virtual Priority GetPriority() const noexcept = 0;
};
