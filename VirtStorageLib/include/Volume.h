#pragma once

#include "Types.h"
#include "../src/VolumeNodeImpl.h"
#include "../src/utils/Noncopyable.h"


//
// Volume
//

template<typename KeyT, typename ValueHolderT = ValueVariant>
class Volume final:
    private NonCopyable
{
public:
    using VolumeNodeImplType = VolumeNodeImpl<KeyT, ValueHolderT>;
    using VolumeNodeImplPtr = std::shared_ptr<VolumeNodeImplType>;
    
    using NodePtr = typename IVolumeNode<KeyT, ValueHolderT>::NodePtr;

public:
    Volume(Priority priority, const std::string& name, const std::string& rootName) : 
        m_priority{ priority }, 
        m_name{ name }, 
        m_root(VolumeNodeImplType::CreateInstance(rootName, priority))
    {
    }

    const std::string& GetName() const noexcept
    {
        return m_name;
    }

    Priority GetPriority() const noexcept
    {
        return m_priority;
    }

    NodePtr GetRoot()
    {
        return m_root->GetProxy();
    };

private:
    VolumeNodeImplPtr m_root;

    Priority m_priority;
    std::string m_name;
};