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


//
// VolumeNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl final :
	public VolumeNodeBaseImpl<KeyT, ValueHolderT>,
	public IProxyProvider<IVolumeNode<KeyT, ValueHolderT>>,
	public INodeInternal,
	public std::enable_shared_from_this<VolumeNodeImpl<KeyT, ValueHolderT>>
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

	static VolumeNodeImplPtr CreateInstance(const std::string& name, Priority priority)
	{
		return std::shared_ptr<VolumeNodeImpl>(new VolumeNodeImpl(name, priority));
	}

	// INode
	const std::string& GetName() const noexcept override
	{
		return m_name;
	}

	void Insert(const KeyT& key, const ValueHolderT& value) override
	{
		m_dict.insert_or_assign(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value) override
	{
		m_dict.insert_or_assign(key, move(value));
	}

	virtual void Erase(const KeyT& key) override
	{
		m_dict.erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		auto it = m_dict.find(key);
		if (it == m_dict.end())
			return false;

		value = it->second;

		return true;
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		auto resIt = m_dict.insert({ key, value });
		return resIt.second;
	}
	
	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		auto resIt = m_dict.insert({ key, value });
		return resIt.second;
	}

	void ForEachKeyValue(ForEachKeyValueFunctorType f) override
	{
		std::for_each(m_dict.begin(), m_dict.end(), [f](auto it)
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
	NodePtr AddChild(std::string name) override
	{
		m_children.push_back(CreateInstance(name, GetPriority()));
		auto child = m_children.back();
		
		m_subscriberHolder.OnNodeAdded(child->GetProxy());

		return child->GetProxy();
	}

	void ForEachChild(ForEachFunctorType f) override
	{
		std::for_each(m_children.begin(), m_children.end(),
			[f](auto node)
			{
				f(node->GetProxy());
			});
	}
	
	NodePtr FindChildIf(FindIfFunctorType f)  override
	{
		auto findIt = std::find_if(m_children.begin(), m_children.end(), 
			[f](auto node)
			{
				return f(node->GetProxy());
			});
		
		if (findIt != m_children.end())
			return (*findIt)->GetProxy();
		
		return nullptr;
	}
	
	void RemoveChildIf(RemoveIfFunctorType f)  override
	{
		for (auto it = m_children.begin(); it != m_children.end();)
		{
			auto node = *it;
			if (f(node->GetProxy()))
			{
				//m_subscriberHolder.OnNodeRemoved(node);
				
				node->MakeOrphan();
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
			[this]()
			{
				m_proxy = VolumeNodeProxyImpl<KeyT, ValueHolderT>::CreateInstance(this->shared_from_this());
			});

		return m_proxy;
	}

	// INodeInternal
	virtual void MakeOrphan() override
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
	VolumeNodeImpl(const std::string& name, Priority priority) : m_name{ name }, m_priority{ priority }
	{
	}

private:
	using DictType = std::unordered_map<KeyT, ValueHolderT>;

	mutable DictType m_dict;
	Priority m_priority;
	std::string m_name;

private:
	using ContainerType = std::list<VolumeNodeImplPtr>;
	ContainerType m_children;

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

	SubscriberHolder m_subscriberHolder;

	private:
		std::shared_ptr<VolumeNodeProxyImpl<KeyT, ValueHolderT>> m_proxy;
		std::once_flag m_createProxyFlag;
};