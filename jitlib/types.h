#pragma once

#include <stdint.h>

using TargetAddress = uint16_t;
using TargetAddressSize = uint16_t;
using NativeAddress = uint8_t *;
using NativeAddressSize = ptrdiff_t;

inline auto ToNativeAddress(void *p) -> NativeAddress 
{
    return reinterpret_cast<NativeAddress>(p);
}

