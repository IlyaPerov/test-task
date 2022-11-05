#pragma once

#include "Types.h"
#include "../src/VirtualNodeImpl.h"

//
// Storage
//

template<typename KeyT, typename ValueHolderT = ValueVariant>
class Storage final
{
public:
    using VirtualNodeImplType = VirtualNodeImpl<KeyT, ValueHolderT>;
    using NodePtr = typename VirtualNodeImplType::NodePtr;

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
    };

private:
    NodePtr m_root;

    std::string m_name;
};