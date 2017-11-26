#pragma once

#include <stdint.h>
#include <vector>

#include "types.h"

class JitVM;

// in order of instruction encoding index
enum X86Register {
    EAX = 0,
    ECX = 1,
    EDX = 2,
    EBX = 3,
    ESP = 4,
    NOREG = 4,  // for MOD R/M displacement only
    EBP = 5,
    ESI = 6,
    EDI = 7,
};

enum X86Register8 {
    AL = 0,
    CL = 1,
    DL = 2,
    BL = 3,
    AH = 4,
    CH = 5,
    DH = 6,
    BH = 7,
};

enum X86Flags {
    X86_CARRY = 0x0001,
    X86_ZERO = 0x0040,
    X86_SIGN = 0x0080,
    X86_OVERFLOW = 0x0800,
};

class AssemblerX86
{
public:
    AssemblerX86(JitVM *vm);

    static_assert(sizeof(NativeAddress) == sizeof(uint32_t), "assembler_x86 may only be compiled in 32-bit.");

    auto beginCodeFragment()->void;
    auto endCodeFragment()->void *;

    auto encodeAndReg8Constant(X86Register8 reg, uint8_t constant)->void;
    auto encodeCall(NativeAddress fn)->void;
    auto encodeJumpIndirect(X86Register reg, uint32_t offset = 0)->void;
    auto encodeLAHF()->void;
    auto encodeMoveRegReg(X86Register dst, X86Register src)->void;
    auto encodeMoveRegReg8(X86Register8 dst, X86Register8 src)->void;
    auto encodeMoveRegPtrOffset(X86Register dst, X86Register ptr, uint32_t offset = 0)->void;
    auto encodeMoveReg8PtrOffset(X86Register8 dst, X86Register ptr, uint32_t offset = 0)->void;
    auto encodeMoveRegConstant(X86Register dst, uint32_t c = 0)->void;
    auto encodeMoveReg8Constant(X86Register8 reg, uint8_t data)->void;
    auto encodeOrRegReg8(X86Register8 dst, X86Register8 src)->void;
    auto encodePopRegister(X86Register reg)->void;
    auto encodePushRegister(X86Register reg)->void;
    auto encodeRet()->void;
    auto encodeShiftRightReg(X86Register reg, uint8_t shift)->void;
    auto encodeXchgReg8(X86Register8 reg1, X86Register8 reg2)->void;
    auto encodeXorReg(X86Register dst, X86Register src)->void;


private:
    enum MOD {
        MOD_INDIRECT = 0x00,
        MOD_DISP8 = 0x40,
        MOD_DISP32 = 0x80,
        MOD_REG = 0xC0,
    };

    auto buildModRM(MOD mod, unsigned reg, unsigned mem)->uint8_t;
       
    JitVM *vm_;
    uint8_t rex_;
};

