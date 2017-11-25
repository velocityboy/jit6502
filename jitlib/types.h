#pragma once

#include <stdint.h>

using Address = uint16_t;
using AddressSize = uint16_t;
using NativeAddress = void *;

inline auto ptrdiff(NativeAddress left, NativeAddress right) -> ptrdiff_t
{
    return reinterpret_cast<char*>(left) - reinterpret_cast<char*>(right);
}
