#pragma once

#include <functional>

struct INodeLifespan
{
    virtual ~INodeLifespan() = default;

    // returns true if a node presents in hierarchy
    virtual bool Exists() const noexcept = 0;

};