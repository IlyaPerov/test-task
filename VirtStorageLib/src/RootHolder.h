#pragma once

#include "../src/utils/Noncopyable.h"
#include <mutex>

namespace vs
{


template<typename NodeImplT>
class RootHolder final :
	private utils::NonCopyable
{
public:
	using NodeImplType = NodeImplT;
	using NodeImplPtr = std::shared_ptr<NodeImplType>;

	using NodeType = typename NodeImplType::NodeType;
	using NodePtr = typename NodeImplType::NodePtr;

public:
	RootHolder() = default;
	RootHolder(RootHolder&&) = default;

	template<typename... ArgsT>
	RootHolder(ArgsT&&... args) :
		m_root(NodeImplType::CreateInstance(std::forward<ArgsT>(args)...))
	{
	}

	RootHolder& operator = (RootHolder&&) = default;

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
		return m_root ? m_root->GetProxy() : nullptr;
	}

private:
	void FreeRootImpl()
	{
		if (m_root)
		{
			m_root->MakeOrphan();
			m_root = nullptr;
		}
	}

	NodeImplPtr m_root;
	mutable std::mutex m_mutex;
};

} //namespace vs