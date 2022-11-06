#pragma once

template<typename ProxiedObjectT>
struct IProxyProvider
{

    virtual ~IProxyProvider() = default;

    virtual typename ProxiedObjectT::NodePtr GetProxy() = 0;
};