#pragma once

#include "VirtualNodeBase.h"
#include "NodeProxyBase.h"

namespace vs
{

namespace internal
{

template<typename KeyT, typename ValueHolderT>
class VirtualNodeImpl;

template<typename KeyT, typename ValueHolderT>
class VirtualNodeProxyImpl final :
	public NodeProxyBaseImpl<
	internal::VirtualNodeBase,
	VirtualNodeImpl,
	KeyT,
	ValueHolderT>
{
public:
	using NodeProxyBaseImplType = NodeProxyBaseImpl<
		internal::VirtualNodeBase,
		VirtualNodeImpl,
		KeyT, ValueHolderT>;

	using NodeType = typename NodeProxyBaseImplType::NodeType;

	using VirtualNodeImplWeakPtr = typename NodeProxyBaseImplType::NodeImplWeakPtr;

	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using ForEachMountedFunctorType = typename IVirtualNode<KeyT, ValueHolderT>::ForEachMountedFunctorType;
	using FindMountedIfFunctorType = typename IVirtualNode<KeyT, ValueHolderT>::FindMountedIfFunctorType;
	using UnmountIfFunctorType = typename IVirtualNode<KeyT, ValueHolderT>::UnmountIfFunctorType;


	VirtualNodeProxyImpl(VirtualNodeImplWeakPtr owner, NodeId nodeId) : NodeProxyBaseImplType(owner, nodeId)
	{
	}

	static std::shared_ptr<NodeType> CreateInstance(VirtualNodeImplWeakPtr owner, NodeId nodeId)
	{
		return std::make_shared<VirtualNodeProxyImpl>(owner, nodeId);
	}

private:

	// IVirtualNodeMounter
	bool Mount(VolumeNodePtr node) override
	{
		return NodeProxyBaseImplType::GetOwner()->Mount(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		NodeProxyBaseImplType::GetOwner()->Unmount(node);
	}

	void ForEachMounted(const ForEachMountedFunctorType& f) const override
	{
		NodeProxyBaseImplType::GetOwner()->ForEachMounted(f);
	}

	VolumeNodePtr FindMountedIf(const FindMountedIfFunctorType& f) const override
	{
		return NodeProxyBaseImplType::GetOwner()->FindMountedIf(f);
	}

	void UnmountIf(const UnmountIfFunctorType& f) override
	{
		NodeProxyBaseImplType::GetOwner()->UnmountIf(f);
	}
};

} //namespace internal

} //namespace vs