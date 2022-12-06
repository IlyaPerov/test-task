#pragma once

#include <list>
#include <unordered_set>
#include <cassert>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

#include "VolumeNode.h"
#include "NodeLifespan.h"
#include "Types.h"

#include "NodeIdImpl.h"
#include "ActionOnRemovedNodeException.h"
#include "InsertInEmptyVirtualNodeException.h"

#include "utils/NonCopyable.h"

#include "intfs/NodeEvents.h"

namespace vs
{

namespace internal
{

// Forward declarations
template<typename KeyT, typename ValueHolderT>
class VirtualNodeImpl;

template<typename KeyT, typename ValueHolderT>
struct IVirtualNodeImplInternal;

namespace virtual_node_details
{


// Helper macros for handling ActionOnRemovedNodeException raised by VolumeNode
#define REMOVED_NODE_EXCEPTION_TRY \
try \
{

#define REMOVED_NODE_EXCEPTION_CATCH \
} \
catch (const ActionOnRemovedNodeException& e) \
{

#define REMOVED_NODE_EXCEPTION_CATCH_END \
}

#define REMOVED_NODE_EXCEPTION_EMPTY_HANDLER \
REMOVED_NODE_EXCEPTION_CATCH \
REMOVED_NODE_EXCEPTION_CATCH_END


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
	using VirtualNodeImplInternalType = IVirtualNodeImplInternal<KeyT, ValueHolderT>;
	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using Ptr = std::shared_ptr<NodeMountAssistant>;

public:

	// helper functions 
	static NodeId GetNodeId(const VolumeNodePtr& volumeNode)
	{
		const auto nodeIdGetter = dynamic_cast<INodeId*>(volumeNode.get());
		assert(nodeIdGetter);
		return nodeIdGetter->GetId();
	}

public:
	static Ptr CreateInstance(VirtualNodeImplType* owner, VolumeNodePtr volumeNode)
	{
		// cannot use make_shared without ugly tricks because of private ctor
		return std::shared_ptr<NodeMountAssistant>(new NodeMountAssistant(owner, volumeNode));
	}

	VolumeNodePtr GetNode() const
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

	Priority GetPriority() const noexcept
	{
		if (!m_volumeNode)
			return MINIMAL_PRIORITY;

		REMOVED_NODE_EXCEPTION_TRY
			return m_volumeNode->GetPriority();
		REMOVED_NODE_EXCEPTION_EMPTY_HANDLER

		return MINIMAL_PRIORITY;
	}

	void Mount()
	{
		assert(m_subscriptionCookie == INVALID_COOKIE);

		std::shared_ptr<INodeEventsSubscription<VolumeNodeType>> subscription = std::dynamic_pointer_cast<INodeEventsSubscription<VolumeNodeType>>(m_volumeNode);
		assert(subscription);

		m_subscriptionCookie = subscription->RegisterSubscriber(this->shared_from_this());

		MountChildren();
	}

	void Unmount()
	{
		assert(m_subscriptionCookie != INVALID_COOKIE);

		REMOVED_NODE_EXCEPTION_TRY
			std::shared_ptr<INodeEventsSubscription<VolumeNodeType>> subscription = std::dynamic_pointer_cast<INodeEventsSubscription<VolumeNodeType>>(m_volumeNode);
			assert(subscription);
			subscription->UnregisterSubscriber(m_subscriptionCookie);
		REMOVED_NODE_EXCEPTION_EMPTY_HANDLER

		UnmountChildren();
		m_volumeNode = nullptr;
	}

private:

	NodeMountAssistant(VirtualNodeImplType* owner, VolumeNodePtr volumeNode) : m_owner{ owner }, m_volumeNode{ volumeNode }
	{
	}

	// INodeEvents
	void OnNodeAdded(VolumeNodePtr node) override
	{
		std::lock_guard lock(m_mutex);

		MountChild(std::move(node));
	}

	void OnNodeRemoved(VolumeNodePtr node) override
	{
		std::lock_guard lock(m_mutex);

		auto it = m_nodes.find(GetNodeId(node));
		if (it != m_nodes.end())
		{
			const auto& pair = it->second;
			pair.virtualNode->Unmount(pair.volumeNode);
			m_nodes.erase(it);
		}
	}


private:
	void MountChildren()
	{
		std::lock_guard lock(m_mutex);

		m_volumeNode->ForEachChild(
			[this](auto node)
			{
				MountChild(std::move(node));
			});
	}

	void UnmountChildren()
	{
		std::lock_guard lock(m_mutex);

		for (auto virtualNodeForVolumeNode : m_nodes)
		{
			const auto& pair = virtualNodeForVolumeNode.second;
			pair.virtualNode->Unmount(pair.volumeNode);
		}

		m_nodes.clear();
	}

	void MountChild(VolumeNodePtr&& child)
	{
		auto virtualNodeAcceptor = static_cast<VirtualNodeImplInternalType*>(m_owner)->InsertChildForMounting(child->GetName());
		if (virtualNodeAcceptor->Mount(child))
			m_nodes.try_emplace(GetNodeId(child), std::move(virtualNodeAcceptor), std::move(child));
	}

private:
	struct VirtualNodeForVolumeNode
	{
		VirtualNodeForVolumeNode(VirtualNodePtr&& virtualNode, VolumeNodePtr&& volumeNode) :
			virtualNode{ std::move(virtualNode) }, volumeNode{ std::move(volumeNode) }
		{}

		VirtualNodePtr virtualNode;
		VolumeNodePtr volumeNode;
	};

	using NodesContainer = std::unordered_map<NodeId, VirtualNodeForVolumeNode>;

	VirtualNodeImplType* m_owner = nullptr;
	VolumeNodePtr m_volumeNode;

	NodesContainer m_nodes;
	Cookie m_subscriptionCookie{ INVALID_COOKIE };
	std::mutex m_mutex;

};

// Helper macro for handling ActionOnRemovedNodeException raised by VolumeNode
#define REMOVED_NODE_EXCEPTION_INVALIDATE_HANDLER \
REMOVED_NODE_EXCEPTION_CATCH \
Invalidate(InvalidReason::NodeUnmounted); \
REMOVED_NODE_EXCEPTION_CATCH_END

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
	using VirtualNodeImplInternalType = typename NodeMountAssistantType::VirtualNodeImplInternalType;

	using ForEachMountedFunctorType = typename VirtualNodeImplType::ForEachMountedFunctorType;
	using FindMountedIfFunctorType = typename VirtualNodeImplType::FindMountedIfFunctorType;
	using UnmountIfFunctorType = typename VirtualNodeImplType::UnmountIfFunctorType;

	using ForEachKeyValueFunctorType = typename VirtualNodeImplType::ForEachKeyValueFunctorType;

public:
	VirtualNodeMounter(VirtualNodeImplType* owner) : m_owner{ owner }
	{
	}


	template<typename T>
	void Insert(const KeyT& key, T&& value)
	{
		std::lock_guard lock(m_mutex);

		Validate();

		if (ReplaceImpl(key, std::forward<T>(value)))
			return;

		for (auto it = m_assistants.begin(); it != m_assistants.end(); it++)
		{
			auto& assistant = *it;
			const auto assistantNode = assistant->GetNode();

			REMOVED_NODE_EXCEPTION_TRY
				assistantNode->Insert(key, std::forward<T>(value));
			REMOVED_NODE_EXCEPTION_CATCH
				// failed to insert because of removed node,
				// continue searching
				continue;
			REMOVED_NODE_EXCEPTION_CATCH_END

			// successful insertion; return 
			return;
		}

		// we have to notify a caller that insertion cannot be done: no actual mounted nodes
		throw InsertInEmptyVirtualNodeException();
	}

	void Erase(const KeyT& key)
	{
		std::lock_guard lock(m_mutex);

		Validate();

		for (auto& assistant : m_assistants)
		{
			REMOVED_NODE_EXCEPTION_TRY
				assistant->GetNode()->Erase(key);
			REMOVED_NODE_EXCEPTION_INVALIDATE_HANDLER
		}
	}

	bool Find(const KeyT& key, ValueHolderT& value) const
	{
		std::lock_guard lock(m_mutex);

		Validate();

		for (auto& assistant : m_assistants)
		{
			REMOVED_NODE_EXCEPTION_TRY
				if (assistant->GetNode()->Find(key, value))
					return true;
			REMOVED_NODE_EXCEPTION_INVALIDATE_HANDLER
		}

		return false;
	}

	bool Contains(const KeyT& key) const
	{
		std::lock_guard lock(m_mutex);

		return ContainsImpl(key);
	}

	template<typename T>
	bool TryInsert(const KeyT& key, T&& value)
	{
		std::lock_guard lock(m_mutex);

		Validate();

		if (ContainsImpl(key))
			return false;

		for (auto it = m_assistants.begin(); it != m_assistants.end(); it++)
		{
			auto& assistant = *it;
			const auto assistantNode = assistant->GetNode();

			REMOVED_NODE_EXCEPTION_TRY
				assistantNode->TryInsert(key, std::forward<T>(value));
			REMOVED_NODE_EXCEPTION_CATCH
				// failed to insert because of removed node,
				// continue searching
				continue;
			REMOVED_NODE_EXCEPTION_CATCH_END

			// successful insertion; return 
			return true;
		}

		// we have to notify a caller that insertion cannot be done: no actual mounted nodes
		throw InsertInEmptyVirtualNodeException();
	}

	template<typename T>
	bool Replace(const KeyT& key, T&& value)
	{
		std::lock_guard lock(m_mutex);

		return ReplaceImpl(key, std::forward<T>(value));
	}

	void ForEachKeyValue(const ForEachKeyValueFunctorType& f)
	{
		std::lock_guard lock(m_mutex);

		Validate();

		using KeysSet = std::unordered_set<KeyT>;
		KeysSet keysCache;

		for (auto& assistant : m_assistants)
		{
			const auto node = assistant->GetNode();
			KeysSet currentKeysCache;
			REMOVED_NODE_EXCEPTION_TRY
				node->ForEachKeyValue(
					[this, &f, &keysCache, &currentKeysCache](const KeyT& key, ValueHolderT& value)
					{
						if (keysCache.count(key))
							return;
						f(key, value);
						currentKeysCache.insert(key);
					});
			REMOVED_NODE_EXCEPTION_CATCH
				if (e.TargetNodeId() != NodeMountAssistantType::GetNodeId(node))
					throw; // it's not our node exception, rethrow it to caller

				Invalidate(InvalidReason::NodeUnmounted);
				continue;
			REMOVED_NODE_EXCEPTION_CATCH_END

			keysCache.insert(currentKeysCache.begin(), currentKeysCache.end());
		}

	}

	// INodeMounter

	bool Mount(VolumeNodePtr node) override
	{
		std::lock_guard m_lock(m_mutex);

		if (FindMountedNode(node))
			return false; // already mounted

		auto assistant = NodeMountAssistantType::CreateInstance(m_owner, node);
		m_assistants.push_back(assistant);
		assistant->Mount();
		Invalidate(InvalidReason::NodeMounted);

		return true;
	}

	void Unmount(VolumeNodePtr node) override
	{
		std::lock_guard m_lock(m_mutex);

		const NodeId nodeToRemoveId = NodeMountAssistantType::GetNodeId(node);

		for (auto it = m_assistants.begin(); it != m_assistants.end(); it++)
		{
			auto& assistant = *it;
			const auto assistantNode = assistant->GetNode();
			if (nodeToRemoveId == NodeMountAssistantType::GetNodeId(assistantNode))
			{
				DoUnmount(it);
				break;
			}
		}

		if (m_assistants.size() == 0)
			static_cast<VirtualNodeImplInternalType*>(m_owner)->OnEntirelyOnmounted();
	}

	void ForEachMounted(const ForEachMountedFunctorType& f) const override
	{
		std::shared_lock m_lock(m_mutex);

		std::for_each(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				if (assistant->HasAliveNode())
					f(assistant->GetNode());
			}
		);
	}

	VolumeNodePtr FindMountedIf(const FindMountedIfFunctorType& f) const override
	{
		std::shared_lock m_lock(m_mutex);

		auto it = std::find_if(m_assistants.begin(), m_assistants.end(),
			[&f](auto assistant)
			{
				if (!assistant->HasAliveNode())
					return false;
				return f(assistant->GetNode());
			});

		if (it == m_assistants.end())
			return nullptr;

		return (*it)->GetNode();
	}

	void UnmountIf(const UnmountIfFunctorType& f) override
	{
		std::lock_guard m_lock(m_mutex);

		for (auto it = m_assistants.begin(); it != m_assistants.end();)
		{
			auto& assistant = *it;
			const auto assistantNode = assistant->GetNode();
			if (f(assistantNode))
				it = DoUnmount(it);
			else
				it++;
		}

		if (m_assistants.size() == 0)
			static_cast<VirtualNodeImplInternalType*>(m_owner)->OnEntirelyOnmounted();
	}

	bool IsEntirelyUnmounted() const
	{
		auto res = true;

		std::lock_guard m_lock(m_mutex);
		for (auto assistant : m_assistants)
		{
			if (assistant->HasAliveNode())
			{
				res = false;
				break;
			}

			Invalidate(InvalidReason::NodeUnmounted);
		}

		return res;
	}

private:
	using AssistantsContainer = std::list<NodeMountAssistantPtr>;

	enum class InvalidReason : uint8_t
	{
		Valid = 0,
		NodeUnmounted,
		NodeMounted
	};

	NodeMountAssistantPtr FindAssistantForNode(NodeId id) const
	{
		auto it = std::find_if(m_assistants.begin(), m_assistants.end(),
			[this, id](auto& assistant)
			{
				REMOVED_NODE_EXCEPTION_TRY
					const auto node = assistant->GetNode();
					if (!node)
						return false;
					if (NodeMountAssistantType::GetNodeId(node) == id)
						return true;
				REMOVED_NODE_EXCEPTION_INVALIDATE_HANDLER

			return false;
			});

		if (it != m_assistants.end())
			return (*it);

		return nullptr;
	}

	VolumeNodePtr FindMountedNode(NodeId id) const
	{
		if (auto assistant = FindAssistantForNode(id))
			return assistant->GetNode();

		return nullptr;
	}

	VolumeNodePtr FindMountedNode(const VolumeNodePtr& volumeNode) const
	{
		if (auto assistant = FindAssistantForNode(NodeMountAssistantType::GetNodeId(volumeNode)))
			return assistant->GetNode();

		return nullptr;
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

		for (auto assistantIt = m_assistants.begin(); assistantIt != m_assistants.end();)
		{
			if (!(*assistantIt)->HasAliveNode())
				assistantIt = m_assistants.erase(assistantIt);
			else
				assistantIt++;
		}

		if (m_invalidReason == InvalidReason::NodeMounted)
			m_assistants.sort(
				[](const NodeMountAssistantPtr& left, const NodeMountAssistantPtr& right)
				{
					return left->GetPriority() > right->GetPriority();
				});

		m_invalidReason = InvalidReason::Valid;

	}

	bool IsValid() const
	{
		return m_invalidReason == InvalidReason::Valid;
	}

	typename AssistantsContainer::iterator DoUnmount(typename AssistantsContainer::iterator it)
	{
		(*it)->Unmount();
		Invalidate(InvalidReason::NodeUnmounted);

		return m_assistants.erase(it);
	}

	template<typename T>
	bool ReplaceImpl(const KeyT& key, T&& value)
	{
		Validate();

		for (auto& assistant : m_assistants)
		{
			REMOVED_NODE_EXCEPTION_TRY
				if (assistant->GetNode()->Replace(key, std::forward<T>(value)))
					return true;
			REMOVED_NODE_EXCEPTION_INVALIDATE_HANDLER
		}

		return false;
	}

	bool ContainsImpl(const KeyT& key) const
	{
		Validate();

		for (auto& assistant : m_assistants)
		{
			REMOVED_NODE_EXCEPTION_TRY
				if (assistant->GetNode()->Contains(key))
					return true;
			REMOVED_NODE_EXCEPTION_INVALIDATE_HANDLER
		}

		return false;
	}

private:

	VirtualNodeImplType* m_owner = nullptr;
	mutable AssistantsContainer m_assistants;
	mutable InvalidReason m_invalidReason = InvalidReason::Valid;
	mutable std::shared_mutex m_mutex;
};

} // namespace virtual_node_details

} //namespace internal

} //namespace vs