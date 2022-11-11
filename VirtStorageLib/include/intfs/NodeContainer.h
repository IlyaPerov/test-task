#pragma once

#include <memory>
#include <functional>

template<typename NodeT>
struct INodeContainer
{
    using NodePtr = std::shared_ptr<NodeT>;
    using NodeWeakPtr = typename std::shared_ptr<NodeT>::weak_type;
    
    using ForEachFunctorType = std::function<void(NodePtr)>;
    using FindIfFunctorType = std::function<bool(NodePtr)>;
    using RemoveIfFunctorType = FindIfFunctorType;

    virtual ~INodeContainer() = default;

    virtual NodePtr AddChild(std::string name) = 0;
    virtual void ForEachChild(ForEachFunctorType f) = 0;
    virtual NodePtr FindChildIf(FindIfFunctorType f) = 0;
    virtual void RemoveChildIf(RemoveIfFunctorType f) = 0;
};