#pragma once

namespace vs
{

namespace internal
{

struct IProxy
{
    virtual ~IProxy() = default;
    
    virtual void Disconnect() noexcept = 0;
};

template<typename ProxiedNodeT>
struct IProxyProvider
{
    virtual ~IProxyProvider() = default;

    virtual typename ProxiedNodeT::NodePtr GetProxy() = 0;
};

} //namespace internal

} //namespace vs