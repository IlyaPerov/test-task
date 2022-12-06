#pragma once

#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

#include "Types.h"
#include "VolumeNode.h"
#include "intfs/ProxyProvider.h"
#include "intfs/NodeInternal.h"

#include "VolumeNodeBase.h"
#include "VolumeNodeProxyImpl.h"
#include "NodeIdImpl.h"


namespace vs
{

namespace internal
{

//
// VolumeNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl final :
	public NodeIdImpl<VolumeNodeBase<KeyT, ValueHolderT>>,
	public IProxyProvider<IVolumeNode<KeyT, ValueHolderT>>,
	public INodeInternal,
	public std::enable_shared_from_this<VolumeNodeImpl<KeyT, ValueHolderT>>
{

public:
	using VolumeNodeBaseType = NodeIdImpl < VolumeNodeBase<KeyT, ValueHolderT>>;
	using VolumeNodeImplType = VolumeNodeImpl<KeyT, ValueHolderT>;
	using VolumeNodeImplPtr = std::shared_ptr<VolumeNodeImplType>;

	using NodeType = IVolumeNode<KeyT, ValueHolderT>;

	using typename NodeType::ForEachKeyValueFunctorType;

	using typename INodeContainer<NodeType>::NodePtr;

	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

	using typename INodeEventsSubscription<NodeType>::NodeEventsPtr;

public:

	static VolumeNodeImplPtr CreateInstance(std::string name, Priority priority)
	{
		return std::shared_ptr<VolumeNodeImpl>(new VolumeNodeImpl(std::move(name), priority));
	}

	// INode
	const std::string& GetName() const noexcept override
	{
		return m_name;
	}

	void Insert(const KeyT& key, const ValueHolderT& value) override
	{
		std::lock_guard lock(m_dictMutex);

		InsertImpl(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value) override
	{
		std::lock_guard lock(m_dictMutex);

		InsertImpl(key, std::move(value));
	}

	void Erase(const KeyT& key) override
	{
		std::lock_guard lock(m_dictMutex);

		m_dict.erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		std::shared_lock lock(m_dictMutex);

		auto it = FindImpl(key);
		if (it == m_dict.end())
			return false;

		value = it->second;

		return true;
	}

	bool Contains(const KeyT& key) const override
	{
		std::shared_lock lock(m_dictMutex);

		auto it = FindImpl(key);
		if (it == m_dict.end())
			return false;

		return true;
	}

	bool TryInsert(const KeyT& key, const ValueHolderT& value) override
	{
		std::lock_guard lock(m_dictMutex);

		return TryInsertImpl(key, value);
	}

	bool TryInsert(const KeyT& key, ValueHolderT&& value) override
	{
		std::lock_guard lock(m_dictMutex);

		return TryInsertImpl(key, std::move(value));
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		std::lock_guard lock(m_dictMutex);

		return ReplaceImpl(key, value);
	}

	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		std::lock_guard lock(m_dictMutex);

		return ReplaceImpl(key, std::move(value));
	}

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f) override
	{
		std::lock_guard lock(m_dictMutex);

		std::for_each(m_dict.begin(), m_dict.end(), [&f](auto& it)
			{
				f(it.first, it.second);
			}
		);
	}

	Priority GetPriority() const noexcept override
	{
		return m_priority;
	}

	// INodeContainer
	NodePtr InsertChild(const std::string& name) override
	{
		NodePtr newChild;

		{
			std::lock_guard lock(m_nodeMutex);

			// "Find-then-insert" instead of "insert-then-test" to avoid
			// possibly redundant calls CreateInstance

			auto it = m_children.find(name);
			if (it != m_children.end())
				newChild = it->second->GetProxy();
			else
			{
				const auto insertRes = m_children.insert({ name, CreateInstance(name, m_priority) });
				newChild = insertRes.first->second->GetProxy();
			}
		}

		m_subscriberHolder.OnNodeAdded(newChild);

		return newChild;
	}

	void ForEachChild(const ForEachFunctorType& f) const  override
	{
		std::shared_lock lock(m_nodeMutex);

		std::for_each(m_children.begin(), m_children.end(),
			[&f](const auto& nameNodePair)
			{
				f(nameNodePair.second->GetProxy());
			});
	}

	NodePtr FindChild(const std::string& name) const override
	{
		std::shared_lock lock(m_nodeMutex);

		const auto it = m_children.find(name);

		if (it != m_children.end())
			return it->second;

		return nullptr;
	}

	NodePtr FindChildIf(const FindIfFunctorType& f) const override
	{
		std::shared_lock lock(m_nodeMutex);

		const auto findIt = std::find_if(m_children.begin(), m_children.end(),
			[&f](const auto& nameNodePair)
			{
				return f(nameNodePair.second->GetProxy());
			});

		if (findIt != m_children.end())
			return findIt->second->GetProxy();

		return nullptr;
	}

	void RemoveChild(const std::string& name) override
	{
		std::lock_guard lock(m_nodeMutex);

		auto it = m_children.find(name);
		if (it == m_children.end())
			return;

		const auto node = it->second;
		m_children.erase(it);

		//TODO: consider calling outside lock
		DoRemoveChild(node);
	}

	void RemoveChildIf(const RemoveIfFunctorType& f)  override
	{
		std::lock_guard lock(m_nodeMutex);

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			const auto node = it->second;
			if (f(node->GetProxy()))
			{
				it = m_children.erase(it);
				//TODO: consider calling outside lock
				DoRemoveChild(node);
			}
			else
				it++;
		}
	}


private:
	// INodeEventsSubscription
	Cookie RegisterSubscriber(NodeEventsPtr subscriber) override
	{
		return m_subscriberHolder.Add(subscriber);
	}

	void UnregisterSubscriber(Cookie cookie) override
	{
		m_subscriberHolder.Remove(cookie);
	}

	// IProxyProvider
	NodePtr GetProxy() override
	{
		return VolumeNodeProxyImpl<KeyT, ValueHolderT>::CreateInstance(this->shared_from_this(), VolumeNodeBaseType::GetId());
	}

	void MakeOrphan() override
	{
		ContainerType childrenCopy;
		{
			std::lock_guard lock(m_nodeMutex);
			childrenCopy = m_children;
			m_children.clear();
		}

		for (const auto& nodeNamePair : childrenCopy)
		{
			const auto& node = nodeNamePair.second;
			m_subscriberHolder.OnNodeRemoved(node);
			node->MakeOrphan();
		}
	}

private:
	using DictType = std::unordered_map<KeyT, ValueHolderT>;
	using ContainerType = std::unordered_map<std::string, VolumeNodeImplPtr>;


private:
	VolumeNodeImpl(std::string name, Priority priority) :
		m_priority{ priority }, m_name {std::move(name)	}
	{
	}

	template<typename T>
	void InsertImpl(const KeyT& key, T&& value)
	{
		m_dict.insert_or_assign(key, std::forward<T>(value));
	}

	template<typename T>
	bool TryInsertImpl(const KeyT& key, T&& value)
	{
		auto resIt = m_dict.try_emplace(key, std::forward<T>(value));
		return resIt.second;
	}

	template<typename T>
	bool ReplaceImpl(const KeyT& key, T&& value)
	{
		auto resIt = m_dict.find(key);
		if (resIt == m_dict.end())
			return false;

		resIt->second = std::forward<T>(value);
		return true;
	}

	typename DictType::const_iterator FindImpl(const KeyT& key) const
	{
		return m_dict.find(key);
	}

	void DoRemoveChild(const VolumeNodeImplPtr& child)
	{
		m_subscriberHolder.OnNodeRemoved(child->GetProxy());

		// It's needed because we want to notify "virtual clients" about removed nodes ASAP
		// Actually different strategies can be applied for making "virtual clients" actual
		child->MakeOrphan();
	}

private:
	class SubscriberHolder :
		public INodeEvents<IVolumeNode<KeyT, ValueHolderT>>
	{
	public:
		Cookie Add(NodeEventsPtr subscriber)
		{
			std::lock_guard lock(m_mutex);
			m_subscribers[m_currentCookie] = subscriber;
			return m_currentCookie++;
		}

		void Remove(Cookie cookie)
		{
			std::lock_guard lock(m_mutex);
			m_subscribers.erase(cookie);
		}

		void Clear()
		{
			std::lock_guard lock(m_mutex);
			m_subscribers.clear();
		}

		void OnNodeAdded(NodePtr node) override
		{
			SubscribersContainerType subscribersCopy;
			{
				std::lock_guard lock(m_mutex);
				subscribersCopy = m_subscribers;
			}

			for (auto subscriber : subscribersCopy)
				subscriber.second->OnNodeAdded(node);
		}

		void OnNodeRemoved(NodePtr node) override
		{
			SubscribersContainerType subscribersCopy;
			{
				std::lock_guard lock(m_mutex);
				subscribersCopy = m_subscribers;
			}

			for (auto subscriber : subscribersCopy)
				subscriber.second->OnNodeRemoved(node);
		}
	private:
		using SubscribersContainerType = std::unordered_map<Cookie, NodeEventsPtr>;
		SubscribersContainerType m_subscribers;
		Cookie m_currentCookie = 1;
		std::mutex m_mutex;
	};

private:
	DictType m_dict;
	Priority m_priority;
	std::string m_name;

	ContainerType m_children;
	SubscriberHolder m_subscriberHolder;

	mutable std::shared_mutex m_dictMutex;
	mutable std::shared_mutex m_nodeMutex;
};

} //namespace internal

} //namespace vs