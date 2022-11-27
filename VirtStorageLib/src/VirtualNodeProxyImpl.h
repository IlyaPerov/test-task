#pragma once

#include "VirtualNodeBaseImpl.h"
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
	internal::VirtualNodeBaseImpl,
	VirtualNodeImpl,
	KeyT,
	ValueHolderT>
{
public:
	using NodeProxyBaseImplType = NodeProxyBaseImpl<
		internal::VirtualNodeBaseImpl,
		VirtualNodeImpl,
		KeyT, ValueHolderT>;

	using VirtualNodeImplWeakPtr = typename NodeProxyBaseImplType::NodeImplWeakPtr;

	using VolumeNodeType = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodeType;
	using VolumeNodePtr = typename VirtualNodeImpl<KeyT, ValueHolderT>::VolumeNodePtr;

	using ForEachMountedFunctorType = typename IVirtualNode<KeyT, ValueHolderT>::ForEachMountedFunctorType;
	using FindMountedIfFunctorType = typename IVirtualNode<KeyT, ValueHolderT>::FindMountedIfFunctorType;
	using UnmountIfFunctorType = typename IVirtualNode<KeyT, ValueHolderT>::UnmountIfFunctorType;


	VirtualNodeProxyImpl(VirtualNodeImplWeakPtr owner) : NodeProxyBaseImplType(owner)
	{
	}

	static std::shared_ptr<VirtualNodeProxyImpl> CreateInstance(VirtualNodeImplWeakPtr owner)
	{
		return std::make_shared<VirtualNodeProxyImpl>(owner);
	}

private:

	// IVirtualNodeMounter
	void Mount(VolumeNodePtr node) override
	{
		GetOwner()->Mount(node);
	}

	void Unmount(VolumeNodePtr node) override
	{
		GetOwner()->Unmount(node);
	}

	void ForEachMounted(const ForEachMountedFunctorType& f) override
	{
		GetOwner()->ForEachMounted(f);
	}

	VolumeNodePtr FindMountedIf(const FindMountedIfFunctorType& f) override
	{
		return GetOwner()->FindMountedIf(f);
	}

	void UnmountIf(const UnmountIfFunctorType& f) override
	{
		GetOwner()->UnmountIf(f);
	}
};

} //namespace internal

} //namespace vs