#pragma once

#include <unordered_map>
#include <algorithm>
#include <mutex>

#include "Types.h"
#include "VolumeNode.h"
#include "intfs/ProxyProvider.h"
#include "intfs/NodeInternal.h"

#include "VolumeNodeBaseImpl.h"
#include "VolumeNodeProxyImpl.h"
#include "NodeIdImpl.h"

#include "utils/NameRegistrar.h"

namespace vs
{

namespace internal
{

//
// VolumeNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl final :
	public NodeIdImpl<VolumeNodeBaseImpl<KeyT, ValueHolderT>>,
	public IProxyProvider<IVolumeNode<KeyT, ValueHolderT>>,
	public INodeInternal,
	public std::enable_shared_from_this<VolumeNodeImpl<KeyT, ValueHolderT>>,
	private utils::NameRegistrar
{

public:
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
		InsertImpl(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value) override
	{
		InsertImpl(key, std::move(value));
	}

	void Erase(const KeyT& key) override
	{
		m_dict.erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		auto it = FindImpl(key);
		if (it == m_dict.end())
			return false;

		value = it->second;

		return true;
	}

	bool Contains(const KeyT& key) const override
	{
		auto it = FindImpl(key);
		if (it == m_dict.end())
			return false;

		return true;
	}

	bool TryInsert(const KeyT& key, const ValueHolderT& value) override
	{
		return TryInsertImpl(key, value);
	}

	bool TryInsert(const KeyT& key, ValueHolderT&& value) override
	{
		return TryInsertImpl(key, std::move(value));
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		return ReplaceImpl(key, value);
	}

	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		return ReplaceImpl(key, std::move(value));
	}

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f) override
	{
		std::for_each(m_dict.begin(), m_dict.end(), [&f](auto it)
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
	NodePtr AddChild(const std::string& name) override
	{
		if (TryAddName(name) == false)
		{
			//TODO: exception?
			return nullptr;
		}

		m_children.push_back(CreateInstance(name, GetPriority()));
		const auto& child = m_children.back();

		m_subscriberHolder.OnNodeAdded(child->GetProxy());

		return child->GetProxy();
	}

	void ForEachChild(const ForEachFunctorType& f) override
	{
		std::for_each(m_children.begin(), m_children.end(),
			[&f](const auto& node)
			{
				f(node->GetProxy());
			});
	}

	NodePtr FindChildIf(const FindIfFunctorType& f) override
	{
		auto findIt = std::find_if(m_children.begin(), m_children.end(),
			[&f](const auto& node)
			{
				return f(node->GetProxy());
			});

		if (findIt != m_children.end())
			return (*findIt)->GetProxy();

		return nullptr;
	}

	void RemoveChildIf(const RemoveIfFunctorType& f)  override
	{
		for (auto it = m_children.begin(); it != m_children.end();)
		{
			const auto& node = *it;
			if (f(node->GetProxy()))
			{
				//TODO: consider to remove the event
				//m_subscriberHolder.OnNodeRemoved(node);

				node->MakeOrphan();
				RemoveName((*it)->m_name);
				it = m_children.erase(it);
			}
			else
				it++;
		}
	}

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
		std::call_once(m_createProxyFlag,
			[this]
			{
				m_proxy = VolumeNodeProxyImpl<KeyT, ValueHolderT>::CreateInstance(this->shared_from_this());
			});

		return m_proxy;
	}

	// INodeInternal
	void MakeOrphan() override
	{
		if (m_proxy)
		{
			m_proxy->Disconnect();
			m_proxy = nullptr;
		}

		for (auto child : m_children)
			child->MakeOrphan();
	}

	// INodelifespan
	bool Exists() const noexcept
	{
		return m_proxy != nullptr;
	}

private:
	using DictType = std::unordered_map<KeyT, ValueHolderT>;
	using ContainerType = std::list<VolumeNodeImplPtr>;


private:
	VolumeNodeImpl(std::string name, Priority priority) :
		m_name{ std::move(name) }, m_priority{ priority }
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

private:
	class SubscriberHolder :
		public INodeEvents<IVolumeNode<KeyT, ValueHolderT>>
	{
	public:
		Cookie Add(NodeEventsPtr subscriber)
		{
			m_subscribers[m_currentCookie] = subscriber;
			return m_currentCookie++;
		}

		void Remove(Cookie cookie)
		{
			m_subscribers.erase(cookie);
		}

		void Clear()
		{
			m_subscribers.clear();
		}

		void OnNodeAdded(NodePtr node) override
		{
			for (auto subscriber : m_subscribers)
				subscriber.second->OnNodeAdded(node);
		}

		void OnNodeRemoved(NodePtr node) override
		{
			for (auto subscriber : m_subscribers)
				subscriber.second->OnNodeRemoved(node);
		}
	private:
		using SubscribersContainerType = std::unordered_map<Cookie, NodeEventsPtr>;
		SubscribersContainerType m_subscribers;
		Cookie m_currentCookie = 1;
	};

private:
	DictType m_dict;
	Priority m_priority;
	std::string m_name;

	ContainerType m_children;

	std::shared_ptr<VolumeNodeProxyImpl<KeyT, ValueHolderT>> m_proxy;
	std::once_flag m_createProxyFlag;
	SubscriberHolder m_subscriberHolder;
};

} //namespace internal

} //namespace vs