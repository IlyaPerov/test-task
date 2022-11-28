#pragma once

#include <mutex>

namespace vs
{

namespace internal
{

template<typename NodeImplT>
class RootHolder final
{
public:
	using NodeImplType = NodeImplT;
	using NodeImplPtr = std::shared_ptr<NodeImplType>;

	using NodeType = typename NodeImplType::NodeType;
	using NodePtr = typename NodeImplType::NodePtr;

public:
	RootHolder() = default;
	~RootHolder() = default;

	RootHolder(const RootHolder&) = delete;
	RootHolder& operator=(const RootHolder&) = delete;

	RootHolder(RootHolder&& other) noexcept : m_root{ std::move(other.m_root) }
	{
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

} //namespace internal

} //namespace vs