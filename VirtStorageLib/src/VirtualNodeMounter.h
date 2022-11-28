#pragma once

#include <list>
#include <unordered_set>
#include <cassert>
#include <algorithm>

#include "VolumeNode.h"
#include "Types.h"

#include "NodeIdImpl.h"
#include "ActionOnRemovedNodeException.h"

#include "utils/NonCopyable.h"

#include "intfs/NodeEvents.h"

namespace vs
{

namespace internal
{

// Forward declarations
template<typename KeyT, typename ValueHolderT>
class VirtualNodeImpl;

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

	bool HasAliveNode() const noexcept
	{
		if (!m_volumeNode)
			return false;

		auto nodeLifespan = std::dynamic_pointer_cast<INodeLifespan>(m_volumeNode);
		assert(nodeLifespan);

		return nodeLifespan->Exists();
	}

	void Mount()
	{
		assert(m_state == MountState::NeedToMount);
		assert(m_subscriptionCookie == INVALID_COOKIE);
		m_state = MountState::Mounted;

		std::shared_ptr<INodeEventsSubscription<VolumeNodeType>> subscription = std::dynamic_pointer_cast<INodeEventsSubscription<VolumeNodeType>>(m_volumeNode);
		assert(subscription);

		m_subscriptionCookie = subscription->RegisterSubscriber(this->shared_from_this());

		MountChildren();
	}

	void Unmount()
	{
		assert(m_state == MountState::Mounted);
		assert(m_subscriptionCookie != INVALID_COOKIE);

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

	NodeMountAssistant(VirtualNodeImplType* owner, VolumeNodePtr volumeNode) : m_owner{ owner }, m_volumeNode{ volumeNode }
	{
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

		m_nodes.clear();
	}

private:
	enum class MountState : uint8_t
	{
		Mounted,
		Unmounted,
		NeedToMount
	};

	struct VirtualNodeForVolumeNode
	{
		VirtualNodePtr virtualNode;
		VolumeNodePtr volumeNode;
	};

	VirtualNodeImplType* m_owner = nullptr;
	VolumeNodePtr m_volumeNode;

	std::vector<VirtualNodeForVolumeNode> m_nodes;
	std::atomic<MountState> m_state{ MountState::NeedToMount };
	Cookie m_subscriptionCookie{ INVALID_COOKIE };

};

//
// VirtualNodeMounter
// 

template<typename KeyT, typename ValueHolderT>
class VirtualNodeMounter :
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
	VirtualNodeMounter(VirtualNodeImplType* owner) : m_owner{ owner }
	{
	}

	void AddNode(VolumeNodePtr volumeNode)
	{
		auto assistant = NodeMountAssistantType::CreateInstance(m_owner, volumeNode);
		m_assistants.push_back(assistant);
		assistant->Mount();
		Invalidate(InvalidReason::NodeMounted);
	}

	void RemoveNode(VolumeNodePtr volumeNode)
	{
		NodeId nodeId;
		try
		{
			nodeId = dynamic_cast<INodeId*>(volumeNode.get())->GetId();
		}
		catch (ActionOnRemovedNodeException&)
		{
			return;
		}

		for (auto it = m_assistants.begin(); it != m_assistants.end(); it++)
		{
			auto& assistant = *it;
			try
			{
				if (nodeId == dynamic_cast<INodeId*>(assistant->GetNode().get())->GetId())
				{
					assistant->Unmount();
					m_assistants.erase(it);
					Invalidate(InvalidReason::NodeUnmouted);
					return;
				}
			}
			catch (ActionOnRemovedNodeException&)
			{
				Invalidate(InvalidReason::NodeUnmouted);
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
				Invalidate(InvalidReason::NodeUnmouted);
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
				Invalidate(InvalidReason::NodeUnmouted);
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
				Invalidate(InvalidReason::NodeUnmouted);
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
				Invalidate(InvalidReason::NodeUnmouted);
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
				Invalidate(InvalidReason::NodeUnmouted);
			}
		}

		return false;
	}

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f)
	{
		Validate();

		using KeysSet = std::unordered_set<KeyT>;
		KeysSet keysCache;

		for (auto& assistant : m_assistants)
		{
			KeysSet currentKeysCache;
			try
			{
				assistant->GetNode()->ForEachKeyValue(
					[this, &f, &keysCache, &currentKeysCache](const KeyT& key, ValueHolderT& value)
					{
						if (keysCache.count(key))
							return;
						f(key, value);
						currentKeysCache.insert(key);
					});
			}
			catch (ActionOnRemovedNodeException&)
			{
				Invalidate(InvalidReason::NodeUnmouted);
				continue;
			}

			keysCache.insert(currentKeysCache.begin(), currentKeysCache.end());

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

		(*it)->Unmount();
		m_assistants.erase(it);
		Invalidate(InvalidReason::NodeUnmouted);
	}

	bool IsEntirelyUnmounted()
	{
		auto res = true;

		for (auto assistant : m_assistants)
		{
			if (assistant->HasAliveNode())
			{
				res = false;
				break;
			}
			else
			{
				Invalidate(InvalidReason::NodeUnmouted);
			}
		}

		return res;
	}

private:
	enum class InvalidReason : uint8_t
	{
		Valid = 0,
		NodeUnmouted,
		NodeMounted
	};

	static NodeId GetNodeId(VolumeNodeType* volumeNode)
	{
		return dynamic_cast<INodeId*>(volumeNode)->GetId();
	}

	VolumeNodePtr FindMountedNode(NodeId id)
	{
		auto it = std::find_if(m_assistants.begin(), m_assistants.end(),
			[this, id](auto& assistant)
			{
				try
				{
					auto node = assistant->GetNode();
					if (!node)
						return false;
					if (GetNodeId(node.get()) == id)
						return true;
				}
				catch (ActionOnRemovedNodeException&)
				{
					Invalidate(InvalidReason::NodeUnmouted);
				}

				return false;
			});

		if (it != m_assistants.end())
			return (*it)->GetNode();

		return nullptr;
	}

	VolumeNodePtr FindMountedNode(VolumeNodeType* volumeNode)
	{
		return FindMountedNode(GetNodeId(volumeNode));
	}

	void Invalidate(InvalidReason reason) const
	{
		if (reason > m_invalidReason)
			m_invalidReason = reason;
	}

	void Validate() const
	{
		if (IsValid())
			return;

		m_invalidReason = InvalidReason::Valid;

		for (auto assistantIt = m_assistants.begin(); assistantIt != m_assistants.end();)
		{
			if(!(*assistantIt)->HasAliveNode())
				assistantIt = m_assistants.erase(assistantIt);
			else
				assistantIt++;
		}

		m_assistants.sort(
			[](const NodeMountAssistantPtr& left, const NodeMountAssistantPtr& right)
			{
				return left->GetNode()->GetPriority() > right->GetNode()->GetPriority();
			});

	}

	bool IsValid() const
	{
		return m_invalidReason == InvalidReason::Valid;
	}

private:
	VirtualNodeImplType* m_owner = nullptr;
	mutable std::list<NodeMountAssistantPtr> m_assistants;
	mutable std::atomic<InvalidReason> m_invalidReason = InvalidReason::Valid;
};

} //namespace internal

} //namespace vs