#pragma once

namespace vs
{

namespace internal
{

struct INodeInternal
{
    virtual ~INodeInternal() = default;
    
    virtual void MakeOrphan() = 0;
};

} //namespace internal

} //namespace vs