#pragma once

#include <vector>

#include "Types.h"


template<typename NodeT, typename ContainerT = std::vector<NodeT>>
class NodeContainerImpl
{
public:
    template <typename... Ts>
    NodeT& AddChild(Ts&&... args)
    {
        m_children.push_back(NodeT(std::forward<Ts...>(args)));
        return m_children.back();
    }

    NodeT& GetChild(size_t index)
    {
        return m_children[index];
    }

    size_t GetChildCount() const noexcept
    {
        return m_children.size();
    }

private:
    ContainerT m_children;
};