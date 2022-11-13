#pragma once

#include <unordered_map>
#include <map>
#include <cassert>

#include "VolumeNode.h"
#include "Types.h"
#include "VirtualNode.h"
#include "utils/Noncopyable.h"
#include "utils/UniqueIdGenerator.h"
#include "intfs/NodeEvents.h"
#include "intfs/NodeId.h"


// Forward declarations:

template<typename KeyT, typename ValueHolderT>
class NodeMountAssistant;

template<typename KeyT, typename ValueHolderT>
class Mounter;


//
// VirtualNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VirtualNodeImpl final :
	public IVirtualNode<KeyT, ValueHolderT>,
	public INodeId,
	public std::enable_shared_from_this<VirtualNodeImpl<KeyT, ValueHolderT>>,
	private NonCopyable
{

public:
	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodeImplPtr = std::shared_ptr<VirtualNodeImplType>;
	using VirtualNodeImplWeakPtr = std::weak_ptr<VirtualNodeImplType>;

	using NodeType = IVirtualNode<KeyT, ValueHolderT>;
	using typename INodeContainer<NodeType>::NodePtr;

	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

	using typename NodeType::ForEachKeyValueFunctorType;

	//using typename IVirtualNode<KeyT, ValueHolderT>::VolumeNodeType;
	//using typename IVirtualNode<KeyT, ValueHolderT>::VolumeNodePtr;
	using VolumeNodeType = IVolumeNode<KeyT, ValueHolderT>;
	using VolumeNodePtr = typename VolumeNodeType::NodePtr;

	using typename IVirtualNode<KeyT, ValueHolderT>::ForEachMountedFunctorType;
	using typename IVirtualNode<KeyT, ValueHolderT>::FindMountedIfFunctorType;
	using typename IVirtualNode<KeyT, ValueHolderT>::UnmountIfFunctorType;

public:

	static VirtualNodeImplPtr CreateInstance(const std::string& name)
	{
		return CreateInstance(name, {});
	}

	// INode
	const std::string& GetName() const noexcept override
	{
		return m_name;
	}

	void Insert(const KeyT& key, const ValueHolderT& value) override
	{
		m_mounter.Insert(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value) override
	{
		m_mounter.Insert(key, value);
	}

	virtual void Erase(const KeyT& key) override
	{
		m_mounter.Erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		return m_mounter.Find(key, value);
	}

	bool Replace(const KeyT& key, const ValueHolderT& value)
	{
		return m_mounter.Replace(key, value);
	}

	bool Replace(const KeyT& key, ValueHolderT&& value)
	{
		return m_mounter.Replace(key, value);
	}

	void ForEachKeyValue(ForEachKeyValueFunctorType f) override
	{
		m_mounter.ForEachKeyValue(f);
	}

	// INodeContainer
	NodePtr AddChild(std::string name) override
	{
		m_mounter.Mount();
		
		m_children.push_back(CreateInstance(name, this->shared_from_this()));
		return m_children.back();
	}

	
	//TODO: Refactor and spread!
	void ForEachChild(ForEachFunctorType f) override
	{
		m_mounter.Mount();

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			auto childNode = *it;

			if (childNode->IsEntirelyUnmounted())
			{
				it = m_children.erase(it);
				continue;
			}
			else
				it++;

			f(childNode);
		}
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
		//m_mounter.AddNode(node);
		m_mounter.Mount(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		m_mounter.Unmount(node);
	}

	void ForEachMounted(ForEachMountedFunctorType f)  override
	{
		m_mounter.ForEachMounted(f);
	}
	
	VolumeNodePtr FindMountedIf(FindMountedIfFunctorType f) override
	{
		return m_mounter.FindMountedIf(f);
	}
	
	void UnmountIf(UnmountIfFunctorType f) override
	{
		m_mounter.UnmountIf(f);
	}

	// INodeId
	NodeId GetId() override
	{
		return m_id;
	}

	void RemoveNodeIfUnmounted(VirtualNodeImpl* node)
	{
		//for (auto it = m_children.begin(); it != m_children.end(); it++)
		//{
		//	if (node->GetId() == (*it)->GetId())
		//	{
		//		if (node->m_mounter.IsUnmounted())
		//		{
		//			m_children.erase(it);
		//			break;
		//		}
		//	}
		//}
	}

private:
	VirtualNodeImpl(const std::string& name, VirtualNodeImplWeakPtr parent = {}) :
		m_name{ name }, m_parent{parent}, m_mounter{ this },
		m_id(UniqueIdGenerator::GetNextUniquId())
	{
	}

	static VirtualNodeImplPtr CreateInstance(const std::string& name, VirtualNodeImplWeakPtr parent)
	{
		return std::shared_ptr<VirtualNodeImpl>(new VirtualNodeImpl(name, parent));
	}

	bool IsEntirelyUnmounted()
	{
		return m_mounter.IsEntirelyUnmounted();
	}

private:
	using ChildrenContainerType = std::list<VirtualNodeImplPtr>;
	using VolumeNodesContainerType = std::vector<VolumeNodePtr>;

	std::string m_name;
	ChildrenContainerType m_children;
	const NodeId m_id;

	mutable Mounter<KeyT, ValueHolderT> m_mounter;

	public:
		VirtualNodeImplWeakPtr m_parent;
};

//
// NodeMountAssistant
// 

template<typename KeyT, typename ValueHolderT>
class NodeMountAssistant:
	public std::enable_shared_from_this<NodeMountAssistant<KeyT, ValueHolderT>>,
	public INodeEvents<IVolumeNode<KeyT, ValueHolderT>>,
	private NonCopyable
{
public:

	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::NodePtr;
	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using Ptr = std::shared_ptr<NodeMountAssistant>;

public:
	~NodeMountAssistant() override
	{
	}
	
	static Ptr CreateInstance(VirtualNodeImplType* owner, VolumeNodePtr volumeNode)
	{
		return std::shared_ptr<NodeMountAssistant>(new NodeMountAssistant(owner, volumeNode));
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

	// INodeEvents
	void OnNodeAdded(VolumeNodePtr node) override
	{
		auto virtualNodeToMountTo = GetOrCreateVirtualChildWithName(node->GetName());

		virtualNodeToMountTo->Mount(node);
		m_nodes.push_back({ virtualNodeToMountTo, node });
	}

	void OnNodeRemoved(VolumeNodePtr node) override
	{
		for (auto virtualNodeForVolumeNode : m_nodes)
		{
			if (virtualNodeForVolumeNode.volumeNode->GetName() == node->GetName())
			{
				virtualNodeForVolumeNode.virtualNode->Unmount(virtualNodeForVolumeNode.volumeNode);
				//m_owner->RemoveNodeIfUnmounted(std::dynamic_pointer_cast<VirtualNodeImplType>(virtualNodeForVolumeNode.virtualNode).get());
			}
		}
	}

private:
	NodeMountAssistant(VirtualNodeImplType* owner, VolumeNodePtr volumeNode) : m_owner{ owner }, m_volumeNode{ volumeNode }
	{
	}

private:

	void Mount()
	{
		assert(m_state == MountState::NeedToMount);
		assert(m_subscriptionCookie == INVALID_COOKIE);

		std::shared_ptr<INodeEventsSubscription<VolumeNodeType>> subscription = std::dynamic_pointer_cast<INodeEventsSubscription<VolumeNodeType>>(m_volumeNode);
				
		m_subscriptionCookie = subscription->RegisterSubscriber(this->shared_from_this());
		
		MountChildren();
		m_state = MountState::Mounted;
	};

	void Unmount()
	{
		assert(m_state == MountState::NeedToUnmount);

		try
		{
			std::shared_ptr<INodeEventsSubscription<VolumeNodeType>> subscription = std::dynamic_pointer_cast<INodeEventsSubscription<VolumeNodeType>>(m_volumeNode);
			assert(subscription);
			subscription->UnregisterSubscriber(m_subscriptionCookie);
		}
		catch (ActionOnRemovedNodeException&)
		{
		}
		
		UnmountChildren();
		m_volumeNode = nullptr;

		m_state = MountState::Unmounted;
	}

private:

	VirtualNodePtr FindVirtualChildWithName(const std::string& name)
	{
		return m_owner->FindChildIf(
			[=](auto virtualChild)
			{
				if (virtualChild->GetName() == name)
					return true;
				return false;
			});
	}

	VirtualNodePtr GetOrCreateVirtualChildWithName(const std::string& name)
	{
		auto virtualChild = FindVirtualChildWithName(name);

		return virtualChild != nullptr ?
			virtualChild :
			m_owner->AddChild(name);
	}

	void MountChildren()
	{
		m_volumeNode->ForEachChild(
			[this](auto node)
			{
				auto virtualNodeToMount = GetOrCreateVirtualChildWithName(node->GetName());

				virtualNodeToMount->Mount(node);
				m_nodes.push_back({ virtualNodeToMount, node });

			});
	}

	void UnmountChildren()
	{
		for (auto virtualNodeForVolumeNode : m_nodes)
			virtualNodeForVolumeNode.virtualNode->Unmount(virtualNodeForVolumeNode.volumeNode);
	}

private:
	enum class MountState : uint8_t
	{
		Mounted,
		Unmounted,
		NeedToMount,
		NeedToUnmount
	};

	struct VirtualNodeForVolumeNode
	{
		VirtualNodePtr virtualNode;
		VolumeNodePtr volumeNode;
	};

	VirtualNodeImplType* m_owner = nullptr;
	VolumeNodePtr m_volumeNode;
	
	std::vector<VirtualNodeForVolumeNode> m_nodes;
	std::atomic<MountState> m_state = MountState::NeedToMount;
	Cookie m_subscriptionCookie = INVALID_COOKIE;

};

//
// Mounter
// 

template<typename KeyT, typename ValueHolderT>
class Mounter:
	public INodeMounter<IVolumeNode<KeyT, ValueHolderT>>
{
public:
	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::NodePtr;
	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using NodeMountAssistantType = NodeMountAssistant<KeyT, ValueHolderT>;
	using NodeMountAssistantTypePtr = typename NodeMountAssistantType::Ptr;

	using ForEachMountedFunctorType = typename VirtualNodeImplType::ForEachMountedFunctorType;
	using FindMountedIfFunctorType = typename VirtualNodeImplType::FindMountedIfFunctorType;
	using UnmountIfFunctorType = typename VirtualNodeImplType::UnmountIfFunctorType;

	using ForEachKeyValueFunctorType = typename VirtualNodeImplType::ForEachKeyValueFunctorType;

public:
	Mounter(VirtualNodeImplType* owner): m_owner{ owner }
	{
	}

	void AddNode(VolumeNodePtr volumeNode)
	{
		//TODO: check name 
		
		m_assistants.push_back(NodeMountAssistantType::CreateInstance(m_owner, volumeNode));
		Invalidate();
	}
	
	void RemoveNode(VolumeNodePtr volumeNode)
	{
		for (auto it = m_assistants.begin(); it != m_assistants.end(); it++)
		{
			auto& assistant = *it;
			if (NodesEqual(volumeNode.get(), assistant->GetNode().get()))
			{
				//TODO: do other stuff
				assistant->PlanUnmounting();
				Invalidate();
				return;
			}
		}
	}

	void Insert(const KeyT& key, const ValueHolderT& value)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Replace(key, value))
					return;
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		if (m_assistants.size() > 0)
			m_assistants.front()->GetNode()->Insert(key, value);
	}

	void Insert(const KeyT& key, ValueHolderT&& value)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Replace(key, value))
					return;
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		if (m_assistants.size() > 0)
			m_assistants.front()->GetNode()->Insert(key, value);
	}

	void Erase(const KeyT& key)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			try
			{
				assistant->GetNode()->Erase(key);
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}
	}

	bool Find(const KeyT& key, ValueHolderT& value) 
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Find(key, value))
					return true;
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		return false;
	}

	bool Replace(const KeyT& key, const ValueHolderT& value)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Replace(key, value))
					return true;
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		return false;
	}

	bool Replace(const KeyT& key, ValueHolderT&& value)
	{
		Validate();
		
		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Replace(key, value))
					return true;
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		return false;
	}

	void ForEachKeyValue(ForEachKeyValueFunctorType f)
	{
		Validate();

		for (auto& assistant : m_assistants)
		{
			try
			{
				assistant->GetNode()->ForEachKeyValue(f);
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

	}

	void Mount()
	{
		Validate();
	}

	// INodeMounter
	
	void Mount(VolumeNodePtr node) override
	{
		AddNode(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		RemoveNode(node);
	}

	void ForEachMounted(ForEachMountedFunctorType f)  override
	{
		std::for_each(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				f(assistant->GetNode());
			}
		);
	}

	VolumeNodePtr FindMountedIf(FindMountedIfFunctorType f) override
	{
		auto it =  std::find_if(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				return f(assistant->GetNode());
			});
		
		if (it == m_assistants.end())
			return nullptr;

		return (*it)->GetNode();
	}

	void UnmountIf(UnmountIfFunctorType f) override
	{
		auto it = std::find_if(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				return f(assistant->GetNode());
			});

		if (it == m_assistants.end())
			return;
		
		(*it)->PlanUnmounting();
		Invalidate();

		//TODO
	}

	bool IsEntirelyUnmounted()
	{
		auto res = true;

		for (auto assistant : m_assistants)
		{
			if (assistant->GetNode()->Exists() == false)
			{
				res = false;
				break;
			}
		}
		
		return res;
	}

private:
	bool NodesEqual(VolumeNodeType* node1, VolumeNodeType* node2)
	{
		if (node1->GetName() == node2->GetName())
			return true;
		
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

		for (auto assistantIt = m_assistants.begin(); assistantIt != m_assistants.end();)
		{
			(*assistantIt)->Validate();
			if ((*assistantIt)->IsUnmounted())
				assistantIt = m_assistants.erase(assistantIt);
			else
				assistantIt++;
		}

		m_assistants.sort( 
			[](NodeMountAssistantTypePtr& left, NodeMountAssistantTypePtr& right) 
			{
				return left->GetNode()->GetPriority() > right->GetNode()->GetPriority();
			});

		//if (IsUnmounted())
		//if (m_assistants.size() == 0)
		//{
		//	//auto parent = m_owner->m_parent.lock();
		//	//if (parent)
		//	//	parent->RemoveNodeIfUnmounted(m_owner);
		//}

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
	mutable std::list<NodeMountAssistantTypePtr> m_assistants;
	mutable std::atomic<ValidatingState> m_state = ValidatingState::Validated;
};