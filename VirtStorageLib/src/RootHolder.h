#pragma once

#include <mutex>
#include "intfs/NodeInternal.h"
#include "intfs/ProxyProvider.h"

namespace vs
{

namespace internal
{

template<typename NodeImplT>
class RootHolder final
{
public:
	using NodeType = typename NodeImplT::NodeType;
	using NodePtr = typename NodeImplT::NodePtr;

public:
	RootHolder() = default;
	~RootHolder()
	{
		FreeRootImpl();
	}

	RootHolder(const RootHolder&) = delete;
	RootHolder& operator=(const RootHolder&) = delete;

	RootHolder(RootHolder&& other) noexcept : m_root{ std::move(other.m_root) }
	{
		static_assert(std::is_base_of_v<IProxyProvider<NodeType>, NodeImplT>, "NodeImplT parameter must derive from IProxyProvider");
	}

	RootHolder& operator = (RootHolder&& other) noexcept
	{
		if (this != &other)
			m_root = std::move(other.m_root);
		return *this;
	}

	template<typename... ArgsT>
	RootHolder(ArgsT&&... args) :
		m_root(NodeImplType::CreateInstance(std::forward<ArgsT>(args)...))
	{
	}

	// Creates a new root; frees the previous one if it exists
	template<typename... ArgsT>
	void CreateRoot(ArgsT&&... args)
	{
		std::lock_guard lock(m_mutex);
		FreeRootImpl();
		m_root = NodeImplType::CreateInstance(std::forward<ArgsT>(args)...);
	}

	void FreeRoot()
	{
		std::lock_guard lock(m_mutex);
		FreeRootImpl();
	}

	NodePtr GetRoot() const
	{
		std::lock_guard lock(m_mutex);
		return m_root ? std::static_pointer_cast<IProxyProvider<NodeType>>(m_root)->GetProxy() : nullptr;
	}

private:
	using NodeImplType = NodeImplT;
	using NodeImplPtr = std::shared_ptr<NodeImplType>;

	void FreeRootImpl()
	{
		if (m_root)
		{
			if constexpr (std::is_base_of_v<INodeInternal, NodeImplType>)
			{
				auto rootInternal = std::static_pointer_cast<INodeInternal>(m_root);
				rootInternal->MakeOrphan();
			}
			m_root = nullptr;
		}
	}

	NodeImplPtr m_root;
	mutable std::mutex m_mutex;
};

} //namespace internal

} //namespace vs