#pragma once

struct INodeInternal
{
    virtual ~INodeInternal() = default;
    
    virtual void MakeOrphan() = 0;
};