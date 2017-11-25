#pragma once

#include <stdint.h>

#include "assembler_x86.h"
#include "types.h"

class JitVM
{
public:
	JitVM(uint32_t reserveSize);
	~JitVM();

    auto beginCodeFragment() -> void;
    auto endCodeFragment() -> NativeAddress;
    auto addByte(uint8_t byte) -> void;
    auto nextByte() -> NativeAddress;

private:
    auto expandRegion() -> void;

	// Base of reserved address space
	uint8_t *regionBase_;

	// Top of reserved address space
	uint8_t *regionTop_;

	// Next unallocated byte
	uint8_t *nextFree_;

	// Top of commited pages region
	uint8_t *regionAllocTop_;

    // The start of the current fragment being constructed (NULL if none)
    uint8_t *currentFragmentStart_;

    // The next available byte of the current fragment being constructed (or NULL)
    uint8_t *nextFragmentByte_;

};