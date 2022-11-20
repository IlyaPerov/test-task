#pragma once

#include "Types.h"
#include "../src/VirtualNodeImpl.h"
#include "../src/utils/Noncopyable.h"

namespace vs
{

//
// Storage
//

template<typename KeyT, typename ValueHolderT = ValueVariant>
class Storage final :
	private utils::NonCopyable
{
public:
	using VirtualNodeImplType = internal::VirtualNodeImpl<KeyT, ValueHolderT>;

	using NodeType = typename IVirtualNode<KeyT, ValueHolderT>::NodeType;
	using NodePtr = typename IVirtualNode<KeyT, ValueHolderT>::NodePtr;

public:
	Storage(const std::string& name, const std::string& rootName) :
		m_name{ name },
		m_root(VirtualNodeImplType::CreateInstance(rootName))
	{
	}

	const std::string& GetName() const noexcept
	{
		return m_name;
	}

	NodePtr GetRoot()
	{
		return m_root;
	}

private:
	NodePtr m_root;

	std::string m_name;
};

} //namespace vs