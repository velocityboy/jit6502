#include "stdafx.h"

#include "jitassembler.h"
#include "jitvm.h"

JitAssembler::JitAssembler(JitVM *vm)
    : vm_(vm)
{
}

auto JitAssembler::beginCodeFragment() -> void
{
    vm_->beginCodeFragment();
}

auto JitAssembler::endCodeFragment() -> void *
{
    return vm_->endCodeFragment();
}

auto JitAssembler::encodeMoveRegReg(X86Register dst, X86Register src)->void
{
    uint8_t modrm = buildModRM(MOD_REG, dst, src);

    vm_->addByte(0x8B);
    vm_->addByte(modrm);
}

auto JitAssembler::encodeMoveRegPtrOffset(X86Register dst, X86Register ptr, uint32_t offset)->void
{
    MOD mod = MOD_INDIRECT;
    int offsetBytes = 0;

    if (offset) {
        if (offset < 0x80 || offset > 0xFFFFFF80) {
            mod = MOD_DISP8;
            offsetBytes = 1;
        }
        else {
            mod = MOD_DISP32;
            offsetBytes = 4;
        }
    }

    uint8_t modrm = buildModRM(mod, dst, ptr);

    vm_->addByte(0x8B);
    vm_->addByte(modrm);

    while (offsetBytes--) {
        vm_->addByte(offset & 0xFF);
        offset >>= 8;
    }
}

auto JitAssembler::encodePushRegister(X86Register reg)->void
{
    vm_->addByte(0x50 | reg);
}

auto JitAssembler::encodeRet() -> void
{
    vm_->addByte(0xC3);
}


auto JitAssembler::buildModRM(MOD mod, X86Register reg, X86Register mem)->uint8_t
{
    // These encodings either mean raw displacement, or the existence of a SIB
    // byte.
    if (mod == MOD_INDIRECT) {
        assert(mem != ESP && mem != EBP);
    }
    else if (mod == MOD_DISP8 || mod == MOD_DISP32) {
        assert(mem != ESP);
    }

    assert(reg >= 0 && reg < 8);
    assert(mem >= 0 && mem < 8);

    return mod | (reg << 3) | mem;
}

