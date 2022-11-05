#pragma once

#include <unordered_map>
#include <map>
#include <cassert>

#include "VolumeNode.h"
#include "Types.h"
#include "VirtualNode.h"



template<typename KeyT, typename ValueHolderT>
class NodeMountAssistant;

template<typename KeyT, typename ValueHolderT>
class Mounter;


//
// VirtualNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VirtualNodeImpl final :
	public IVirtualNode<KeyT, ValueHolderT>
{

public:
	using NodeType = IVirtualNode<KeyT, ValueHolderT>;
	using typename INodeContainer<NodeType>::NodePtr;

	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

	using typename IVirtualNode<KeyT, ValueHolderT>::VolumeNodeType;
	using typename IVirtualNode<KeyT, ValueHolderT>::VolumeNodePtr;

public:

	static NodePtr CreateInstance(const std::string& name)
	{
		return std::make_shared<VirtualNodeImpl>(name);
	}

	VirtualNodeImpl(const std::string& name) : m_name{ name }, m_mounter(this)
	{
	}

	// INode
	const std::string& GetName() const noexcept override
	{
		return m_name;
	}

	void Insert(const KeyT& Key, const ValueHolderT& Value) override
	{
		m_mounter.Insert(Key, Value);
	}

	void Insert(const KeyT& Key, ValueHolderT&& Value) override
	{
		m_mounter.Insert(Key, Value);
	}

	virtual void Erase(const KeyT& Key) override
	{
		m_mounter.Erase(Key);
	}

	bool Find(const KeyT& Key, ValueHolderT& Value) const override
	{
		return m_mounter.Find(Key, Value);
	}

	bool Replace(const KeyT& Key, const ValueHolderT& Value)
	{
		return m_mounter.Replace(Key, Value);
	}

	bool Replace(const KeyT& Key, ValueHolderT&& Value)
	{
		return m_mounter.Replace(Key, Value);
	}

	// INodeContainer
	NodePtr AddChild(const std::string& name) override
	{
		m_mounter.Mount();
		
		m_children.push_back(CreateInstance(name));
		return m_children.back();
	}

	void ForEachChild(ForEachFunctorType f) override
	{
		m_mounter.Mount();
		
		std::for_each(m_children.begin(), m_children.end(), f);
	}

	NodePtr FindChildIf(FindIfFunctorType f)  override
	{
		m_mounter.Mount();
		
		auto findIt = std::find_if(m_children.begin(), m_children.end(), f);
		if (findIt != m_children.end())
			return *m_children.end();

		return nullptr;
	}

	void RemoveChildIf(RemoveIfFunctorType f)  override
	{
		m_mounter.Mount();
		
		auto removedEndIt = std::remove_if(m_children.begin(), m_children.end(), f);

		m_children.erase(removedEndIt, m_children.end());
	}

	// IVirtualNode
	void Mount(VolumeNodePtr node) override
	{
		m_mounter.AddNode(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		m_mounter.RemoveNode(node);
	}

private:
	using ChildrenContainerType = std::vector<NodePtr>;
	using VolumeNodesContainerType = std::vector<VolumeNodePtr>;
	friend NodeMountAssistant;
	
private:
	std::string m_name;
	ChildrenContainerType m_children;
	VolumeNodesContainerType m_volumeNodes;

	Mounter<KeyT, ValueHolderT> m_mounter;
};

//
// NodeMountAssistant
// 

template<typename KeyT, typename ValueHolderT>
class NodeMountAssistant
{
public:

	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::NodePtr;
	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	enum class MountState : uint8_t
	{
		Mounted,
		Unmounted,
		NeedToMount,
		NeedToUnmount
	};


public:
	NodeMountAssistant(VirtualNodeImplType* owner, VolumeNodePtr volumeNode) : m_owner{ owner }, m_volumeNode{ volumeNode }
	{
	}

	VolumeNodePtr GetNode()
	{
		return m_volumeNode;
	};

	void PlanUnmounting()
	{
		m_state = MountState::NeedToUnmount;
	}

	bool IsUnmounted()
	{
		return m_state == MountState::Unmounted;
	}

	void Validate()
	{
		switch (m_state)
		{
		case MountState::NeedToMount:
			Mount();
			break;
		case MountState::NeedToUnmount:
			Unmount();
			break;
			
		case MountState::Unmounted:
		case MountState::Mounted:
			//do nothing: everything is fine
			break;

		default:
			assert(false);
		}
	}

private:

	struct VirtualNodeForVolumeNode
	{
		VirtualNodePtr virtualNode;
		VolumeNodePtr volumeNode;
	};

	void MountChildren()
	{
		m_volumeNode->ForEachChild(
			[this](auto node)
			{
				auto searchResIt = std::find_if(m_owner->m_children.begin(), m_owner->m_children.end(),
					[=](auto virtualChild)
					{
						if (virtualChild->GetName() == node->GetName())
							return true;
						return false;
					});


				VirtualNodePtr virtualNodeToMountTo;
				if (searchResIt != m_owner->m_children.end())
					virtualNodeToMountTo = *searchResIt;
				else
					virtualNodeToMountTo = m_owner->AddChild(node->GetName());

				virtualNodeToMountTo->Mount(node);
				m_nodes.push_back({ virtualNodeToMountTo, node });

			});
	}

	void UnmountChildren()
	{
		for (auto virtualNodeForVolumeNode : m_nodes)
		{
			virtualNodeForVolumeNode.virtualNode->Unmount(virtualNodeForVolumeNode.volumeNode);
		}
	}

	void Mount()
	{
		assert(m_state == MountState::NeedToMount);
		
		MountChildren();
		m_state = MountState::Mounted;
	};

	void Unmount()
	{
		assert(m_state == MountState::NeedToUnmount);
		
		UnmountChildren();
		m_volumeNode = nullptr;

		m_state = MountState::Unmounted;
	}
		 

private:
	VirtualNodeImplType* m_owner = nullptr;
	VolumeNodePtr m_volumeNode;
	
	std::vector<VirtualNodeForVolumeNode> m_nodes;
	std::atomic<MountState> m_state = MountState::NeedToMount;

};

//
// Mounter
// 

template<typename KeyT, typename ValueHolderT>
class Mounter
{
public:
	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::NodePtr;
	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using NodeMountAssistantType = NodeMountAssistant<KeyT, ValueHolderT>;

public:
	Mounter(VirtualNodeImplType* owner): m_owner{ owner }
	{
	}

	void AddNode(VolumeNodePtr volumeNode)
	{
		m_assistants.emplace_back(m_owner, volumeNode);
		Invalidate();
	}
	
	void RemoveNode(VolumeNodePtr volumeNode)
	{
		for (auto it = m_assistants.begin(); it != m_assistants.end(); it++)
		{
			if (NodesEqual(volumeNode.get(), it->GetNode().get()))
			{
				//TODO: do other stuff
				it->PlanUnmounting();
				Invalidate();
				return;
			}
		}
	}

	void Insert(const KeyT& Key, const ValueHolderT& Value)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			if (assistant.GetNode()->Replace(Key, Value))
				return;
		}

		if (m_assistants.size() > 0)
			m_assistants.front().GetNode()->Insert(Key, Value);
	}

	void Insert(const KeyT& Key, ValueHolderT&& Value)
	{
		Validate();
		
		for (auto assistant : m_assistants)
		{
			if (assistant.GetNode()->Replace(Key, Value))
				return;
		}

		if (m_assistants.size() > 0)
			m_assistants[0].GetNode()->Insert(Key, Value);
	}

	void Erase(const KeyT& Key)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
			assistant.GetNode()->Erase(Key);
	}

	bool Find(const KeyT& Key, ValueHolderT& Value) const
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			if (assistant.GetNode()->Find(Key, Value))
				return true;
		}

		return false;
	}

	bool Replace(const KeyT& Key, const ValueHolderT& Value)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			if (assistant.GetNode()->Replace(Key, Value))
				return true;
		}

		return false;
	}

	bool Replace(const KeyT& Key, ValueHolderT&& Value)
	{
		Validate();
		
		for (auto assistant : m_assistants)
		{
			if (assistant.GetNode()->Replace(Key, Value))
				return true;
		}

		return false;
	}

	void Mount()
	{
		Validate();
	}

private:
	bool NodesEqual(VolumeNodeType* node1, VolumeNodeType* node2)
	{
		//TODO:
		
		return false;
	}

	void Invalidate()
	{
		m_state = ValidatingState::Unvalidated;
	}

	void Validate() const
	{
		if (IsValid())
			return;

		m_state = ValidatingState::ValidatingInProgress;

		for (auto assistantIt = m_assistants.begin(); assistantIt != m_assistants.end(); assistantIt++)
		{
			assistantIt->Validate();
			if (assistantIt->IsUnmounted())
				m_assistants.erase(assistantIt);
		}

		m_assistants.sort( 
			[](NodeMountAssistantType& left, NodeMountAssistantType& right) 
			{
				return left.GetNode()->GetPriority() > right.GetNode()->GetPriority();
			});

		m_state = ValidatingState::Validated;
	}

	bool IsValid() const
	{
		auto state = m_state.load();
		return state == ValidatingState::Validated || state == ValidatingState::ValidatingInProgress;
	}

private:
	enum class ValidatingState : uint8_t
	{
		Validated,
		Unvalidated,
		ValidatingInProgress
	};

private:
	VirtualNodeImplType* m_owner = nullptr;
	mutable std::list<NodeMountAssistantType> m_assistants;
	mutable std::atomic<ValidatingState> m_state = ValidatingState::Validated;
};