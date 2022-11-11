#pragma once

#include <unordered_map>
#include <algorithm>

#include "Types.h"
#include "VolumeNode.h"
#include "VolumeNodeBaseImpl.h"
#include "ActionOnRemovedNodeException.h"

#include "./intfs/ProxyProvider.h"


template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl;


//
// VolumeNodeProxyImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeProxyImpl final:
	public VolumeNodeBaseImpl<KeyT, ValueHolderT>,
	public IProxy,
	public std::enable_shared_from_this<VolumeNodeProxyImpl<KeyT, ValueHolderT>>
{

public:
	using VolumeNodeImplType = VolumeNodeImpl<KeyT, ValueHolderT>;
	using VolumeNodeImplPtr = std::shared_ptr<VolumeNodeImplType>;
	using VolumeNodeImplWeakPtr = std::weak_ptr<VolumeNodeImplType>;

	using NodeType = IVolumeNode<KeyT, ValueHolderT>;
	using typename INodeContainer<NodeType>::NodePtr;
	using typename INodeContainer<NodeType>::NodeWeakPtr;

	using typename NodeType::ForEachKeyValueFunctorType;
	
	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

	using typename INodeEventsSubscription<NodeType>::NodeEventsPtr;

public:

	static std::shared_ptr<VolumeNodeProxyImpl> CreateInstance(VolumeNodeImplWeakPtr owner)
	{
		return std::shared_ptr<VolumeNodeProxyImpl>(new VolumeNodeProxyImpl(owner));
	}

	// INode
	const std::string& GetName() const override
	{
		return GetOwner()->GetName();
	}

	void Insert(const KeyT& key, const ValueHolderT& value) override
	{
		GetOwner()->Insert(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value) override
	{
		GetOwner()->Insert(key, value);
	}

	virtual void Erase(const KeyT& key) override
	{
		GetOwner()->Erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		return GetOwner()->Find(key, value);
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		return GetOwner()->Replace(key, value);
	}
	
	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		return GetOwner()->Replace(key, value);
	}

	void ForEachKeyValue(ForEachKeyValueFunctorType f) override
	{
		return GetOwner()->ForEachKeyValue(f);
	}

	Priority GetPriority() const override
	{
		return GetOwner()->GetPriority();
	}

	// INodeContainer
	NodePtr AddChild(std::string name) override
	{
		return GetOwner()->AddChild(name);
	}

	void ForEachChild(ForEachFunctorType f) override
	{
		GetOwner()->ForEachChild(f);
	}
	
	NodePtr FindChildIf(FindIfFunctorType f)  override
	{
		return GetOwner()->FindChildIf(f);
	}
	
	void RemoveChildIf(RemoveIfFunctorType f)  override
	{
		GetOwner()->RemoveChildIf(f);
	}

	// INodeEventsSubscription
	Cookie RegisterSubscriber(NodeEventsPtr subscriber) override
	{
		std::shared_ptr<INodeEventsSubscription<NodeType>> subscription = std::static_pointer_cast<INodeEventsSubscription<NodeType>>(GetOwner());

		return subscription->RegisterSubscriber(subscriber);
	}

	void UnregisterSubscriber(Cookie cookie) override
	{
		std::shared_ptr<INodeEventsSubscription<NodeType>> subscription = std::static_pointer_cast<INodeEventsSubscription<NodeType>>(GetOwner());
		
		return subscription->UnregisterSubscriber(cookie);
	}

	// IProxy
	void Disconnect() noexcept override
	{
		auto owner = m_owner.lock();
		
		m_owner.reset();
	}

private:
	VolumeNodeProxyImpl(VolumeNodeImplWeakPtr owner) : m_owner{ owner }
	{
	}

private:
	VolumeNodeImplPtr GetOwner() const
	{
		auto owner = m_owner.lock();

		if (!owner)
			throw ActionOnRemovedNodeException("Cannot perform an action because the target node  was removed from chierarchy");

		return owner;
	}

private:
	//mutable NodeWeakPtr m_owner;
	mutable VolumeNodeImplWeakPtr m_owner;

};