#pragma once

#include "Types.h"
#include "../src/VolumeNodeImpl.h"
#include "../src/RootHolder.h"

namespace vs
{

template <typename KeyT, typename ValueHolderT = ValueVariant>
using Volume = internal::RootHolder<internal::VolumeNodeImpl<KeyT, ValueHolderT>>;

} //namespace vs