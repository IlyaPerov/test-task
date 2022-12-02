#pragma once

#include <algorithm>

#include "Types.h"
#include "NodeLifespan.h"
#include "ActionOnRemovedNodeException.h"

namespace vs
{

namespace internal
{

//
// NodeProxyBaseImpl
//

template<
	template <typename, typename> typename BaseT,
	template <typename, typename> typename NodeImplT,
	typename KeyT, typename ValueHolderT>
class NodeProxyBaseImpl:
	public BaseT<KeyT, ValueHolderT>,
	public INodeLifespan
{

public:
	using BaseType = BaseT<KeyT, ValueHolderT>;
	using NodeImplType = NodeImplT<KeyT, ValueHolderT>;
	using NodeImplPtr = std::shared_ptr<NodeImplType>;
	using NodeImplWeakPtr = std::weak_ptr<NodeImplType>;

	using NodeType = typename NodeImplType::NodeType;
	using typename INodeContainer<NodeType>::NodePtr;
	using typename INodeContainer<NodeType>::NodeWeakPtr;

	using typename NodeType::ForEachKeyValueFunctorType;
	
	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

public:

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
		GetOwner()->Insert(key, std::move(value));
	}

	virtual void Erase(const KeyT& key) override
	{
		GetOwner()->Erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		return GetOwner()->Find(key, value);
	}

	bool Contains(const KeyT& key) const override
	{
		return GetOwner()->Contains(key);
	}
	
	bool TryInsert(const KeyT& key, const ValueHolderT& value) override
	{
		return GetOwner()->TryInsert(key, value);
	}

	bool TryInsert(const KeyT& key, ValueHolderT&& value) override
	{
		return GetOwner()->TryInsert(key, std::move(value));
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		return GetOwner()->Replace(key, value);
	}

	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		return GetOwner()->Replace(key, std::move(value));
	}

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f) override
	{
		return GetOwner()->ForEachKeyValue(f);
	}

	// INodeContainer
	NodePtr AddChild(const std::string& name) override
	{
		return GetOwner()->AddChild(name);
	}

	void ForEachChild(const ForEachFunctorType& f) override
	{
		GetOwner()->ForEachChild(f);
	}
	
	NodePtr FindChildIf(const FindIfFunctorType& f) override
	{
		return GetOwner()->FindChildIf(f);
	}
	
	void RemoveChildIf(const RemoveIfFunctorType& f) override
	{
		GetOwner()->RemoveChildIf(f);
	}

	// INodeLifespan
	bool Exists() const noexcept
	{
		return !m_owner.expired();
	}

	// INodeId
	NodeId GetId() const override
	{
		return GetOwner()->GetId();
	}


protected:
	NodeProxyBaseImpl(NodeImplWeakPtr owner, NodeId nodeId) : m_owner{ owner }, m_nodeId{ nodeId }
	{
		static_assert(std::is_base_of_v<NodeType, BaseType>, "BaseT parameter must derive from INode");
		static_assert(std::is_base_of_v<INodeContainer<NodeType>, BaseType>, "BaseT parameter must derive from INodeLifespan");
		static_assert(std::is_base_of_v<INodeId, BaseType>, "BaseT parameter must derive from INodeId");
	}

	NodeImplPtr GetOwner() const
	{
		auto owner = m_owner.lock();

		if (!owner)
			throw ActionOnRemovedNodeException(m_nodeId);

		return owner;
	}

private:
	NodeImplWeakPtr m_owner;
	NodeId m_nodeId;

};

} //namespace internal

} //namespace vs