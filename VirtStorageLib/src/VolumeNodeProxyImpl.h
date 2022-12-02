#pragma once

#include "Types.h"
#include "VolumeNodeBase.h"
#include "NodeProxyBase.h"

namespace vs
{

namespace internal
{


template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl;

template<typename KeyT, typename ValueHolderT>
class VolumeNodeProxyImpl final :
	public NodeProxyBaseImpl<
		internal::VolumeNodeBase,
		VolumeNodeImpl,
		KeyT,
		ValueHolderT>
{
public:
	using NodeProxyBaseImplType = NodeProxyBaseImpl<
		internal::VolumeNodeBase,
		VolumeNodeImpl,
		KeyT, ValueHolderT>;

	using NodeType = typename NodeProxyBaseImplType::NodeType;
	using VolumeNodeImplWeakPtr = typename NodeProxyBaseImplType::NodeImplWeakPtr;
	using typename INodeEventsSubscription<NodeType>::NodeEventsPtr;

	VolumeNodeProxyImpl(VolumeNodeImplWeakPtr owner, NodeId nodeId) : NodeProxyBaseImplType(owner, nodeId)
	{
	}

	static std::shared_ptr<NodeType> CreateInstance(VolumeNodeImplWeakPtr owner, NodeId nodeId)
	{
		return std::make_shared<VolumeNodeProxyImpl>(owner, nodeId);
	}

private:

	// IVolumeNode
	Priority GetPriority() const override
	{
		return NodeProxyBaseImplType::GetOwner()->GetPriority();
	}

	// INodeEventsSubscription
	Cookie RegisterSubscriber(NodeEventsPtr subscriber) override
	{
		std::shared_ptr<INodeEventsSubscription<NodeType>> subscription = std::static_pointer_cast<INodeEventsSubscription<NodeType>>(NodeProxyBaseImplType::GetOwner());

		return subscription->RegisterSubscriber(subscriber);
	}

	void UnregisterSubscriber(Cookie cookie) override
	{
		std::shared_ptr<INodeEventsSubscription<NodeType>> subscription = std::static_pointer_cast<INodeEventsSubscription<NodeType>>(NodeProxyBaseImplType::GetOwner());

		return subscription->UnregisterSubscriber(cookie);
	}
};

} //namespace internal

} //namespace vs