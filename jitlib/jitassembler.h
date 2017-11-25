#pragma once

#include <stdint.h>
#include <vector>

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

class JitAssembler
{
public:
    JitAssembler(JitVM *vm);

    auto beginCodeFragment()->void;
    auto endCodeFragment()->void *;

    auto encodeCall(void *fn)->void;
    auto encodeJumpIndirect(X86Register reg, uint32_t offset = 0)->void;
    auto encodeMoveRegReg(X86Register dst, X86Register src)->void;
    auto encodeMoveRegPtrOffset(X86Register dst, X86Register ptr, uint32_t offset = 0)->void;
    auto encodeMoveRegConstant(X86Register dst, uint32_t c = 0)->void;
    auto encodePopRegister(X86Register reg)->void;
    auto encodePushRegister(X86Register reg)->void;
    auto encodeRet()->void;


private:
    enum MOD {
        MOD_INDIRECT = 0x00,
        MOD_DISP8 = 0x40,
        MOD_DISP32 = 0x80,
        MOD_REG = 0xC0,
    };

    auto buildModRM(MOD mod, X86Register reg, X86Register mem)->uint8_t;
       
    JitVM *vm_;
    uint8_t rex_;
};

