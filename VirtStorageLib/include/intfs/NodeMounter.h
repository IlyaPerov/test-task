#pragma once

#include <functional>

namespace vs
{

template<typename MountableNodeT>
struct INodeMounter
{
	using MountableNodeType = MountableNodeT;
	using MountableNodePtr = typename MountableNodeT::NodePtr;

	using ForEachMountedFunctorType = std::function<void(MountableNodePtr)>;
	using FindMountedIfFunctorType = std::function<bool(MountableNodePtr)>;
	using UnmountIfFunctorType = FindMountedIfFunctorType;

	virtual ~INodeMounter() = default;

	virtual void Mount(MountableNodePtr node) = 0;
	virtual void Unmount(MountableNodePtr node) = 0;
	virtual void ForEachMounted(const ForEachMountedFunctorType& f) = 0;
	virtual MountableNodePtr FindMountedIf(const FindMountedIfFunctorType& f) = 0;
	virtual void UnmountIf(const UnmountIfFunctorType& f) = 0;
};

} //namespace vs