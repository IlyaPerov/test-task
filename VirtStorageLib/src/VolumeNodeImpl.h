#pragma once

#include <unordered_map>
#include <algorithm>

#include "Types.h"
#include "VolumeNode.h"
#include "intfs\ProxyProvider.h"

#include "VolumeNodeBaseImpl.h"
#include "VolumeNodeProxyImpl.h"


//
// VolumeNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl final:
	public VolumeNodeBaseImpl<KeyT, ValueHolderT>,
	public IProxyProvider<IVolumeNode<KeyT, ValueHolderT>>,
	public std::enable_shared_from_this<VolumeNodeImpl<KeyT, ValueHolderT>>
{

public:
	using NodeType = IVolumeNode<KeyT, ValueHolderT>;
	using typename INodeContainer<NodeType>::NodePtr;
	
	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

public:

	static NodePtr CreateInstance(const std::string& name, Priority priority)
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

	Priority GetPriority() const noexcept override
	{
		return m_priority;
	}

	// INodeContainer
	NodePtr AddChild(const std::string& name) override
	{
		m_children.push_back(CreateInstance(name, GetPriority()));
		return m_children.back();
	}

	void ForEachChild(ForEachFunctorType f) override
	{
		std::for_each(m_children.begin(), m_children.end(), f);
	}
	
	NodePtr FindChildIf(FindIfFunctorType f)  override
	{
		auto findIt = std::find_if(m_children.begin(), m_children.end(), f);
		if (findIt != m_children.end())
			return *m_children.end();	
		
		return nullptr;
	}
	
	void RemoveChildIf(RemoveIfFunctorType f)  override
	{
		auto removedEndIt = std::remove_if(m_children.begin(), m_children.end(), f);
		
		m_children.erase(removedEndIt, m_children.end());
	}

	// IProxyProvider
	NodePtr GetProxy() override
	{
		return VolumeNodeProxyImpl<KeyT, ValueHolderT>::CreateInstance(this->shared_from_this());
	}

private:
	VolumeNodeImpl(const std::string& name, Priority priority) : m_name{ name }, m_priority{ priority }
	{
	}

	VolumeNodeImpl(const VolumeNodeImpl&) = delete;
	VolumeNodeImpl(VolumeNodeImpl&&) = delete;


private:
	using DictType = std::unordered_map<KeyT, ValueHolderT>;

	mutable DictType m_dict;
	Priority m_priority;
	std::string m_name;

private:
	using ContainerType = std::vector<NodePtr>;
	ContainerType m_children;
};