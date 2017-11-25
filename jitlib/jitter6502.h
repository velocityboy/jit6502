#pragma once

#include "types.h"

#include <array>
#include <unordered_map>

class JitVM;
class JitAssembler;
class SystemMemory;

class Jitter6502
{
public:

    Jitter6502(JitVM *vm, JitAssembler *assembler, SystemMemory *memory);

    auto boot()->void;

    auto jit(Address ip)->NativeAddress;

private:
    using InstructionJitter = bool(Jitter6502::*)(Address *ip);
    using Entry = void(*)(JitVM *, NativeAddress entry);
    using Exit = void(*)(void);

    auto buildReentryStub()->void;
    static auto invalidOpcodeStub(Address addr)->void;
    auto jitInvalidOpcode(Address *ip)->bool;

    static std::array<InstructionJitter, 256> jitters_;

    JitVM *vm_;
    JitAssembler *assembler_;
    SystemMemory *memory_;
    Entry entryStub_;
    Exit exitStub_;
};
