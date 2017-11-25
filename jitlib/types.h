#pragma once

#include <stdint.h>

using TargetAddress = uint16_t;
using TargetAddressSize = uint16_t;
using NativeAddress = void *;

inline auto ptrdiff(NativeAddress left, NativeAddress right) -> ptrdiff_t
{
    return reinterpret_cast<char*>(left) - reinterpret_cast<char*>(right);
}
