#pragma once

#include "types.h"

#include <array>
#include <unordered_map>

class JitVM;
class AssemblerX86;
class SystemMemory;

class Jitter6502
{
public:

    Jitter6502(JitVM *vm, AssemblerX86 *assembler, SystemMemory *memory);

    auto boot()->void;

    auto jit(TargetAddress ip)->NativeAddress;

private:
    using InstructionJitter = bool(Jitter6502::*)(TargetAddress *ip);
    using Entry = void(*)(JitVM *, NativeAddress entry);
    using Exit = void(*)(void);

    auto buildReentryStub()->void;
    static auto invalidOpcodeStub(TargetAddress addr)->void;
    auto jitInvalidOpcode(TargetAddress *ip)->bool;

    static std::array<InstructionJitter, 256> jitters_;

    JitVM *vm_;
    AssemblerX86 *assembler_;
    SystemMemory *memory_;
    Entry entryStub_;
    Exit exitStub_;
};
