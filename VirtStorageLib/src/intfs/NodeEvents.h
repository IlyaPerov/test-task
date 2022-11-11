#pragma once
#include <memory>

#include <Types.h>

template<typename NodeT>
struct INodeEvents
{
    using NodePtr = typename NodeT::NodePtr;

    virtual ~INodeEvents() = default;

    virtual void OnNodeAdded(NodePtr node) = 0;
    virtual void OnNodeRemoved(NodePtr node) = 0;
};


template<typename NodeT>
struct INodeEventsSubscription
{
    using NodeEventsPtr = std::shared_ptr<INodeEvents<NodeT>>;
    
    virtual ~INodeEventsSubscription() = default;

    virtual Cookie RegisterSubscriber(NodeEventsPtr subscriber) = 0;
    virtual void UnregisterSubscriber(Cookie cookie) = 0;
};
