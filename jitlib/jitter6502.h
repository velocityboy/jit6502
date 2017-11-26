#pragma once

#include "types.h"

#include <array>
#include <unordered_map>

class JitVM;
class AssemblerX86;
class SystemMemory;

enum M6502Flags 
{
    M6502_CARRY = 0x01,
    M6502_ZERO = 0x02,
    M6502_INTERRUPT = 0x04,
    M6502_DECIMAL = 0x08,
    M6502_BRK = 0x10,
    M6502_ALWAYS = 0x20,
    M6502_OVERFLOW = 0x40,
    M6502_SIGN = 0x80,
};

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
    using FlagTranslationMap = std::array<uint8_t, 256>;

    auto buildReentryStub()->void;
    auto buildFlagTranslationMap()->void;

    static auto invalidOpcodeStub(TargetAddress addr)->void;
    auto jitInvalidOpcode(TargetAddress *ip)->bool;

    auto jitLDA_IMM(TargetAddress *ip)->bool;

    auto jit_getImmediateIntoAL(TargetAddress *ip)->void;

    auto jit_setFlags(uint8_t mask)->void;

    static std::array<InstructionJitter, 256> jitters_;

    JitVM *vm_;
    AssemblerX86 *assembler_;
    SystemMemory *memory_;
    Entry entryStub_;
    Exit exitStub_;
    FlagTranslationMap flagTranslationMap_;
};
