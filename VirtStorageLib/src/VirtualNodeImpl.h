#pragma once

#include <unordered_map>
#include <map>
#include <cassert>
#include <algorithm>

#include "VolumeNode.h"
#include "Types.h"
#include "VirtualNode.h"

#include "NodeIdImpl.h"
#include "ActionOnRemovedNodeException.h"

#include "utils/Noncopyable.h"
#include "utils/NameRegistrar.h"

#include "intfs/NodeEvents.h"

namespace vs
{

namespace internal
{

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
	public NodeIdImpl<INodeId>,
	public std::enable_shared_from_this<VirtualNodeImpl<KeyT, ValueHolderT>>,
	private utils::NonCopyable,
	private utils::NameRegistrar
{

public:
	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodeImplPtr = std::shared_ptr<VirtualNodeImplType>;

	using NodeType = IVirtualNode<KeyT, ValueHolderT>;
	using typename INodeContainer<NodeType>::NodePtr;

	using typename INodeContainer<NodeType>::ForEachFunctorType;
	using typename INodeContainer<NodeType>::FindIfFunctorType;
	using typename INodeContainer<NodeType>::RemoveIfFunctorType;

	using typename NodeType::ForEachKeyValueFunctorType;

	using VolumeNodeType = IVolumeNode<KeyT, ValueHolderT>;
	using VolumeNodePtr = typename VolumeNodeType::NodePtr;

	using typename IVirtualNode<KeyT, ValueHolderT>::ForEachMountedFunctorType;
	using typename IVirtualNode<KeyT, ValueHolderT>::FindMountedIfFunctorType;
	using typename IVirtualNode<KeyT, ValueHolderT>::UnmountIfFunctorType;


public:

	static VirtualNodeImplPtr CreateInstance(const std::string& name)
	{
		return CreateInstance(name, NodeKind::UserCreated);
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
		m_mounter.Insert(key, std::move(value));
	}

	virtual void Erase(const KeyT& key) override
	{
		m_mounter.Erase(key);
	}

	bool Find(const KeyT& key, ValueHolderT& value) const override
	{
		return m_mounter.Find(key, value);
	}

	bool Contains(const KeyT& key) const override
	{
		return m_mounter.Contains(key);
	}

	bool TryInsert(const KeyT& key, const ValueHolderT& value) override
	{
		return m_mounter.TryInsert(key, value);
	}

	bool TryInsert(const KeyT& key, ValueHolderT&& value) override
	{
		return m_mounter.TryInsert(key, std::move(value));
	}

	bool Replace(const KeyT& key, const ValueHolderT& value) override
	{
		return m_mounter.Replace(key, value);
	}

	bool Replace(const KeyT& key, ValueHolderT&& value) override
	{
		return m_mounter.Replace(key, std::move(value));
	}

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f) override
	{
		m_mounter.ForEachKeyValue(f);
	}

	// INodeContainer
	NodePtr AddChild(const std::string& name) override
	{
		return AddNode(name, NodeKind::UserCreated);
	}

	void ForEachChild(const ForEachFunctorType& f) override
	{
		m_mounter.Mount();

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			if (StripIfUnmounted(it))
				continue;

			f(*it);
			it++;
		}
	}

	NodePtr FindChildIf(const FindIfFunctorType& f) override
	{
		m_mounter.Mount();

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			if (StripIfUnmounted(it))
				continue;

			if (f(*it))
				return *it;

			it++;
		}

		return nullptr;
	}

	void RemoveChildIf(const RemoveIfFunctorType& f)  override
	{
		m_mounter.Mount();

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			if (StripIfUnmounted(it))
				continue;

			if (f(*it))
				it = m_children.erase(it);
			else
				it++;
		}
	}

	// IVirtualNode
	void Mount(VolumeNodePtr node) override
	{
		m_mounter.Mount(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		m_mounter.Unmount(node);
	}

	void ForEachMounted(const ForEachMountedFunctorType& f)  override
	{
		m_mounter.ForEachMounted(f);
	}

	VolumeNodePtr FindMountedIf(const FindMountedIfFunctorType& f) override
	{
		return m_mounter.FindMountedIf(f);
	}

	void UnmountIf(const UnmountIfFunctorType& f) override
	{
		m_mounter.UnmountIf(f);
	}

	NodePtr AddChildForMounting(const std::string& name)
	{
		return AddNode(name, NodeKind::ForMounting);
	}

private:
	using ChildrenContainerType = std::list<VirtualNodeImplPtr>;
	enum class NodeKind
	{
		UserCreated,
		ForMounting
	};

private:

	VirtualNodeImpl(const std::string& name, NodeKind kind) :
		m_name{ name }, m_kind{ kind }, m_mounter{ this }
	{
	}

	static VirtualNodeImplPtr CreateInstance(const std::string& name, NodeKind kind)
	{
		return std::shared_ptr<VirtualNodeImpl>(new VirtualNodeImpl(name, kind));
	}

	bool IsEntirelyUnmounted()
	{
		if (m_kind == NodeKind::UserCreated)
			return false;

		return m_mounter.IsEntirelyUnmounted();
	}

	NodePtr AddNode(const std::string& name, NodeKind kind)
	{
		m_mounter.Mount();

		if (TryAddName(name) == false)
		{
			//TODO: exception?
			return nullptr;
		}

		m_children.push_back(CreateInstance(name, kind));
		return m_children.back();
	}

	bool StripIfUnmounted(typename ChildrenContainerType::iterator& it)
	{
		if ((*it)->IsEntirelyUnmounted())
		{
			it = m_children.erase(it);
			return true;
		}

		return false;

	}

private:

	std::string m_name;
	ChildrenContainerType m_children;
	const NodeKind m_kind;

	mutable Mounter<KeyT, ValueHolderT> m_mounter;
};

//
// NodeMountAssistant
// 

template<typename KeyT, typename ValueHolderT>
class NodeMountAssistant :
	public std::enable_shared_from_this<NodeMountAssistant<KeyT, ValueHolderT>>,
	public INodeEvents<IVolumeNode<KeyT, ValueHolderT>>,
	private utils::NonCopyable
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
	}

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
		//TODO: consider to remove
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
		assert(subscription);

		m_subscriptionCookie = subscription->RegisterSubscriber(this->shared_from_this());

		MountChildren();
		m_state = MountState::Mounted;
	}

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
			[&name](auto virtualChild)
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
			m_owner->AddChildForMounting(name);
	}

	void MountChildren()
	{
		m_volumeNode->ForEachChild(
			[this](auto node)
			{
				auto virtualNodeAcceptor = GetOrCreateVirtualChildWithName(node->GetName());

		virtualNodeAcceptor->Mount(node);
		m_nodes.push_back({ virtualNodeAcceptor, node });

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
class Mounter :
	public INodeMounter<IVolumeNode<KeyT, ValueHolderT>>
{
public:
	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::NodePtr;
	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using NodeMountAssistantType = NodeMountAssistant<KeyT, ValueHolderT>;
	using NodeMountAssistantPtr = typename NodeMountAssistantType::Ptr;

	using ForEachMountedFunctorType = typename VirtualNodeImplType::ForEachMountedFunctorType;
	using FindMountedIfFunctorType = typename VirtualNodeImplType::FindMountedIfFunctorType;
	using UnmountIfFunctorType = typename VirtualNodeImplType::UnmountIfFunctorType;

	using ForEachKeyValueFunctorType = typename VirtualNodeImplType::ForEachKeyValueFunctorType;

public:
	Mounter(VirtualNodeImplType* owner) : m_owner{ owner }
	{
	}

	void AddNode(VolumeNodePtr volumeNode)
	{
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
				assistant->PlanUnmounting();
				Invalidate();
				return;
			}
		}
	}

	template<typename T>
	void Insert(const KeyT& key, T&& value)
	{
		Validate();

		if (Replace(key, std::forward<T>(value)))
			return;

		if (m_assistants.size() > 0)
			m_assistants.front()->GetNode()->Insert(key, std::forward<T>(value));
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

	bool Find(const KeyT& key, ValueHolderT& value) const
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

	bool Contains(const KeyT& key)
	{
		Validate();

		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Contains(key))
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

	template<typename T>
	bool TryInsert(const KeyT& key, T&& value)
	{
		Validate();

		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Contains(key))
					return false;
			}
			catch (ActionOnRemovedNodeException&)
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		if (m_assistants.size() > 0)
			m_assistants.front()->GetNode()->TryInsert(key, std::forward<T>(value));

		return true;
	}

	template<typename T>
	bool Replace(const KeyT& key, T&& value)
	{
		Validate();

		for (auto& assistant : m_assistants)
		{
			try
			{
				if (assistant->GetNode()->Replace(key, std::forward<T>(value)))
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

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f)
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

	void Mount() const
	{
		Validate();
	}

	// INodeMounter

	void Mount(VolumeNodePtr node) override
	{
		if (FindMountedNode(node.get()))
		{
			// The node's already mounted
			//TODO: exception?
			return;
		}

		AddNode(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		RemoveNode(node);
	}

	void ForEachMounted(const ForEachMountedFunctorType& f)  override
	{
		std::for_each(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				f(assistant->GetNode());
			}
		);
	}

	VolumeNodePtr FindMountedIf(const FindMountedIfFunctorType& f) override
	{
		auto it = std::find_if(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				return f(assistant->GetNode());
			});

		if (it == m_assistants.end())
			return nullptr;

		return (*it)->GetNode();
	}

	void UnmountIf(const UnmountIfFunctorType& f) override
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
	}

	bool IsEntirelyUnmounted()
	{
		auto res = true;

		for (auto assistant : m_assistants)
		{
			if (assistant->GetNode()->Exists() == true)
			{
				res = false;
				break;
			}
			else
			{
				assistant->PlanUnmounting();
				Invalidate();
			}
		}

		return res;
	}

private:
	static bool NodesEqual(VolumeNodeType* node1, VolumeNodeType* node2)
	{
		if (dynamic_cast<INodeId*>(node1)->GetId() == dynamic_cast<INodeId*>(node2)->GetId())
			return true;

		return false;
	}

	VolumeNodePtr FindMountedNode(NodeId id)
	{
		auto it = std::find_if(m_assistants.begin(), m_assistants.end(),
			[id](auto& assistant)
			{
				if (std::dynamic_pointer_cast<INodeId>(assistant->GetNode())->GetId() == id)
				return true;

		return false;
			});

		if (it != m_assistants.end())
			return (*it)->GetNode();

		return nullptr;
	}

	VolumeNodePtr FindMountedNode(VolumeNodeType* volumeNode)
	{
		return FindMountedNode(dynamic_cast<INodeId*>(volumeNode)->GetId());
	}

	void Invalidate() const
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
			[](const NodeMountAssistantPtr& left, const NodeMountAssistantPtr& right)
			{
				return left->GetNode()->GetPriority() > right->GetNode()->GetPriority();
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
	mutable std::list<NodeMountAssistantPtr> m_assistants;
	mutable std::atomic<ValidatingState> m_state = ValidatingState::Validated;
};

} //namespace internal

} //namespace vs