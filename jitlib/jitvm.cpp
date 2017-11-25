#include "stdafx.h"

#include <assert.h>
#include <stdexcept>

#include "jitvm.h"

using std::runtime_error;

namespace
{
    const unsigned EXPAND_SIZE = 4096;
}


JitVM::JitVM(uint32_t reserveSize)
{
    regionBase_ = static_cast<uint8_t*>(VirtualAlloc(nullptr, reserveSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (regionBase_ == nullptr) {
        throw runtime_error("Failed to reserve address space for JIT VM");
    }

    regionTop_ = regionBase_ + reserveSize;
    regionAllocTop_ = regionBase_;
    nextFree_ = regionBase_;
    currentFragmentStart_ = nullptr;
    nextFragmentByte_ = nullptr;
 }

JitVM::~JitVM()
{
    VirtualFree(regionBase_, 0, MEM_RELEASE);
}

auto JitVM::beginCodeFragment() -> void
{
    assert(currentFragmentStart_ == nullptr);
    assert(nextFragmentByte_ == nullptr);

    currentFragmentStart_ = nextFree_;
    nextFragmentByte_ = nextFree_;
}

auto JitVM::endCodeFragment() -> NativeAddress
{
    assert(currentFragmentStart_ != nullptr);
    assert(nextFragmentByte_ != nullptr);

    if (!FlushInstructionCache(GetCurrentProcess(), currentFragmentStart_, nextFragmentByte_ - currentFragmentStart_)) {
        throw runtime_error("JitVM failed to flush the instruction cache");
    }
    
    auto start = currentFragmentStart_;

    nextFree_ = nextFragmentByte_;

    currentFragmentStart_ = nullptr;
    nextFragmentByte_ = nullptr;

    return start;
}

auto JitVM::addByte(uint8_t byte) -> void
{
    assert(nextFragmentByte_ <= regionAllocTop_);
    if (nextFragmentByte_ == regionAllocTop_) {
        expandRegion();
    }
    *nextFragmentByte_++ = byte;
}

auto JitVM::nextByte()->NativeAddress
{
    return nextFragmentByte_;
}


auto JitVM::expandRegion() -> void
{
    if (VirtualAlloc(regionAllocTop_, EXPAND_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE) == nullptr) {
        throw runtime_error("JitVM failed to allocate more pages.");
    }
    regionAllocTop_ += EXPAND_SIZE;
}
