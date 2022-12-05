#pragma once

#include <algorithm>

#include "VolumeNode.h"
#include "VirtualNode.h"
#include "intfs/ProxyProvider.h"

#include "VirtualNodeBase.h"
#include "NodeIdImpl.h"
#include "VirtualNodeMounter.h"
#include "VirtualNodeProxyImpl.h"

namespace vs
{

namespace internal
{

//
// IVirtualNodeImplInternal
//

template<typename KeyT, typename ValueHolderT>
struct IVirtualNodeImplInternal
{
	using NodeType = IVirtualNode<KeyT, ValueHolderT>;
	using NodePtr = typename INodeContainer<NodeType>::NodePtr;

	virtual ~IVirtualNodeImplInternal() = default;

	virtual void OnEntirelyOnmounted() = 0;
	virtual NodePtr InsertChildForMounting(const std::string& name) = 0;
};

//
// VirtualNodeImpl
//

template<typename KeyT, typename ValueHolderT>
class VirtualNodeImpl final :
	public NodeIdImpl<VirtualNodeBase<KeyT, ValueHolderT>>,
	public std::enable_shared_from_this<VirtualNodeImpl<KeyT, ValueHolderT>>,
	public IProxyProvider<IVirtualNode<KeyT, ValueHolderT>>,
	public IVirtualNodeImplInternal<KeyT, ValueHolderT>
{

public:
	using VirtualNodeBaseType = NodeIdImpl < VirtualNodeBase<KeyT, ValueHolderT> >;
	using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
	using VirtualNodeImplPtr = std::shared_ptr<VirtualNodeImplType>;
	using VirtualNodeImplWeakPtr = std::weak_ptr<VirtualNodeImplType>;

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

	static VirtualNodeImplPtr CreateInstance(std::string name)
	{
		return CreateInstance(std::move(name), NodeKind::Ordinary, {});
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

	void Erase(const KeyT& key) override
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
	NodePtr InsertChild(const std::string& name) override
	{
		return InsertNode(name, m_kind);
	}

	void ForEachChild(const ForEachFunctorType& f) const override
	{
		std::shared_lock lock(m_nodeMutex);

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			const auto& node = it->second;
			f(node->GetProxy());
			it++;
		}
	}

	NodePtr FindChild(const std::string& name) const override
	{
		std::shared_lock lock(m_nodeMutex);

		auto it = m_children.find(name);
		if (it != m_children.end())
			return it->second;

		return nullptr;
	}

	NodePtr FindChildIf(const FindIfFunctorType& f) const override
	{
		std::shared_lock lock(m_nodeMutex);

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			const auto& node = it->second;
			if (f(node->GetProxy()))
				return node->GetProxy();

			it++;
		}

		return nullptr;
	}

	void RemoveChildIf(const RemoveIfFunctorType& f)  override
	{
		std::lock_guard lock(m_nodeMutex);

		for (auto it = m_children.begin(); it != m_children.end();)
		{
			const auto& node = it->second;
			if (f(node->GetProxy()))
				it = m_children.erase(it);
			else
				it++;
		}
	}

	void RemoveChild(const std::string& name) override
	{
		std::lock_guard lock(m_nodeMutex);

		m_children.erase(name);
	}

	// IVirtualNodeMounter
	bool Mount(VolumeNodePtr node) override
	{
		return m_mounter.Mount(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		m_mounter.Unmount(node);
	}

	void ForEachMounted(const ForEachMountedFunctorType& f) const override
	{
		m_mounter.ForEachMounted(f);
	}

	VolumeNodePtr FindMountedIf(const FindMountedIfFunctorType& f) const override
	{
		return m_mounter.FindMountedIf(f);
	}

	void UnmountIf(const UnmountIfFunctorType& f) override
	{
		m_mounter.UnmountIf(f);
	}

private:
	// IProxyProvider
	NodePtr GetProxy() override
	{
		return VirtualNodeProxyImpl<KeyT, ValueHolderT>::CreateInstance(this->shared_from_this(), VirtualNodeBaseType::GetId());
	}

	// IVirtualNodeImplInternal
	NodePtr InsertChildForMounting(const std::string& name) override
	{
		return InsertNode(name, NodeKind::ForMounting);
	}

	void OnEntirelyOnmounted()
	{
		if (auto parent = m_parent.lock())
			parent->RemoveChild(GetName());
	}

private:
	using ChildrenContainerType = std::unordered_map<std::string, VirtualNodeImplPtr>;
	enum class NodeKind
	{
		Ordinary,
		ForMounting
	};

private:

	VirtualNodeImpl(std::string name, NodeKind kind, VirtualNodeImplWeakPtr parent) :
		m_name{ std::move(name) }, m_kind{ kind }, m_mounter{ this }, m_parent { std::move(parent)}
	{
	}

	static VirtualNodeImplPtr CreateInstance(std::string name, NodeKind kind, VirtualNodeImplWeakPtr parent)
	{
		// cannot use make_shared without ugly tricks because of private ctor
		return std::shared_ptr<VirtualNodeImpl>(new VirtualNodeImpl(std::move(name), kind, std::move(parent)));
	}

	NodePtr InsertNode(const std::string& name, NodeKind kind)
	{
		std::lock_guard lock(m_nodeMutex);

		// "Find-then-insert" instead of "insert-then-test" to avoid
		// possibly redundant calls CreateInstance

		auto it = m_children.find(name);
		if (it != m_children.end())
			return it->second->GetProxy();

		const auto insertRes = m_children.insert({ name, CreateInstance(name, kind, this->shared_from_this()) });
		return insertRes.first->second->GetProxy();
	}

private:

	std::string m_name;
	ChildrenContainerType m_children;
	const NodeKind m_kind;
	virtual_node_details::VirtualNodeMounter<KeyT, ValueHolderT> m_mounter;
	VirtualNodeImplWeakPtr m_parent;
	mutable std::shared_mutex m_nodeMutex;
};

} //namespace internal

} //namespace vs