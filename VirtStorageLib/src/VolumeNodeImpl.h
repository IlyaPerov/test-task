#pragma once

#include <unordered_map>
#include <algorithm>

#include "Types.h"
#include "VolumeNode.h"


//
// VolumeNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VolumeNodeImpl final:
	public IVolumeNode<KeyT, ValueHolderT>
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
		return std::make_shared<VolumeNodeImpl>(name, priority);
	}
	
	VolumeNodeImpl(const std::string& name, Priority priority) : m_name{ name }, m_priority{ priority }
	{
	}

	// INode
	const std::string& GetName() const noexcept override
	{
		return m_name;
	}

	void Insert(const KeyT& Key, const ValueHolderT& Value) override
	{
		m_dict.insert_or_assign(Key, Value);
	}

	void Insert(const KeyT& Key, ValueHolderT&& Value) override
	{
		m_dict.insert_or_assign(Key, move(Value));
	}

	virtual void Erase(const KeyT& Key) override
	{
		m_dict.erase(Key);
	}

	bool Find(const KeyT& Key, ValueHolderT& Value) const override
	{
		auto it = m_dict.find(Key);
		if (it == m_dict.end())
			return false;

		Value = it->second;

		return true;
	}

	bool Replace(const KeyT& Key, const ValueHolderT& Value) override
	{
		auto resIt = m_dict.insert({ Key, Value });
		return resIt.second;
	}
	
	bool Replace(const KeyT& Key, ValueHolderT&& Value) override
	{
		auto resIt = m_dict.insert({ Key, Value });
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

private:
	using DictType = std::unordered_map<KeyT, ValueHolderT>;

	mutable DictType m_dict;
	Priority m_priority;
	std::string m_name;

private:
	using ContainerType = std::vector<NodePtr>;
	ContainerType m_children;
};