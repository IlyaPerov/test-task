#pragma once

#include "Types.h"
#include "../src/VirtualNodeImpl.h"
#include "../src/utils/Noncopyable.h"

#include "Types.h"
#include "../src/VolumeNodeImpl.h"
#include "../src/RootHolder.h"

namespace vs
{

template <typename KeyT, typename ValueHolderT = ValueVariant>
using Storage = internal::RootHolder<internal::VirtualNodeImpl<KeyT, ValueHolderT>>;

} //namespace vs