#pragma once

#include <unordered_map>
#include <algorithm>

#include "Types.h"
#include "VolumeNode.h"
#include "VolumeNodeBaseImpl.h"
#include "ActionOnRemovedNodeException.h"


//
// VolumeNodeProxyImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeProxyImpl final:
	public VolumeNodeBaseImpl<KeyT, ValueHolderT>,
	public std::enable_shared_from_this<VolumeNodeProxyImpl<KeyT, ValueHolderT>>
{

public:
	using NodeType = IVolumeNode<KeyT, ValueHolderT>;
	using typename INodeContainer<NodeType>::NodePtr;
	using typename INodeContainer<NodeType>::NodeWeakPtr;
	
	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

public:

	static NodePtr CreateInstance(NodeWeakPtr owner)
	{
		return std::shared_ptr<VolumeNodeProxyImpl>(new VolumeNodeProxyImpl(owner));
	}

	// INode
	const std::string& GetName() const override
	{
		return LockOwner()->GetName();
	}

	void Insert(const KeyT& key, const ValueHolderT& value) override
	{
		LockOwner()->Insert(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value) override
	{
		LockOwner()->Insert(key, value);
	}

	virtual void Erase(const KeyT& key) override
	{
		LockOwner()->Erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		return LockOwner()->Find(key, value);
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		return LockOwner()->Replace(key, value);
	}
	
	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		return LockOwner()->Replace(key, value);
	}

	Priority GetPriority() const override
	{
		return LockOwner()->GetPriority();
	}

	// INodeContainer
	NodePtr AddChild(const std::string& name) override
	{
		return LockOwner()->AddChild(name);
	}

	void ForEachChild(ForEachFunctorType f) override
	{
		LockOwner()->ForEachChild(f);
	}
	
	NodePtr FindChildIf(FindIfFunctorType f)  override
	{
		return LockOwner()->FindChildIf(f);
	}
	
	void RemoveChildIf(RemoveIfFunctorType f)  override
	{
		LockOwner()->RemoveChildIf(f);
	}

private:
	VolumeNodeProxyImpl(NodeWeakPtr owner) : m_owner{ owner }
	{
	}

	VolumeNodeProxyImpl(const VolumeNodeProxyImpl&) = delete;
	VolumeNodeProxyImpl(VolumeNodeProxyImpl&&) = delete;

private:
	NodePtr LockOwner() const
	{
		auto owner = m_owner.lock();

		if (!owner)
			throw ActionOnRemovedNodeException("Cannot perform an action because the target node  was removed from chierarchy");

		return owner;
	}

private:
	mutable NodeWeakPtr m_owner;

};