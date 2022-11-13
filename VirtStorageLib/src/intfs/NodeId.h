#pragma once

#include "Types.h"

struct INodeId
{
    virtual ~INodeId() = default;
    
    virtual NodeId GetId() = 0;
};