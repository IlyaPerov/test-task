#pragma once

struct IProxy
{
    virtual ~IProxy() = default;
    
    virtual void Disconnect() = 0;
};

template<typename ProxiedNodeT>
struct IProxyProvider
{

    virtual ~IProxyProvider() = default;

    virtual typename ProxiedNodeT::NodePtr GetProxy() = 0;
};