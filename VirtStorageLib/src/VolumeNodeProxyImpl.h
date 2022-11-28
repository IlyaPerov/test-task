#pragma once

#include "Types.h"
#include "VolumeNodeBaseImpl.h"
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
		internal::VolumeNodeBaseImpl,
		VolumeNodeImpl,
		KeyT,
		ValueHolderT>
{
public:
	using NodeProxyBaseImplType = NodeProxyBaseImpl<
		internal::VolumeNodeBaseImpl,
		VolumeNodeImpl,
		KeyT, ValueHolderT>;

	using VolumeNodeImplWeakPtr = typename NodeProxyBaseImplType::NodeImplWeakPtr;
	using NodeType = typename NodeProxyBaseImplType::NodeType;
	using typename INodeEventsSubscription<NodeType>::NodeEventsPtr;

	VolumeNodeProxyImpl(VolumeNodeImplWeakPtr owner) : NodeProxyBaseImplType(owner)
	{
	}

	static std::shared_ptr<VolumeNodeProxyImpl> CreateInstance(VolumeNodeImplWeakPtr owner)
	{
		return std::make_shared<VolumeNodeProxyImpl>(owner);
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