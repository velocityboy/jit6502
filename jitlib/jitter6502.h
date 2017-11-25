#pragma once

#include <array>

#include "types.h"

class JitVM;
class JitAssembler;
class SystemMemory;

class Jitter6502
{
public:
    Jitter6502(JitVM *vm, JitAssembler *assembler, SystemMemory *memory);

    auto jit(Address ip)->void;

private:
    using InstructionJitter = bool(Jitter6502::*)();

    auto buildReentryStub()->void;
    auto jitInvalidOpcode()->bool;

    static std::array<InstructionJitter, 256> jitters_;

    JitVM *vm_;
    JitAssembler *assembler_;
    SystemMemory *memory_;
};
