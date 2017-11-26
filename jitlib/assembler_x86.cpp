#include "stdafx.h"

#include "assembler_x86.h"
#include "jitvm.h"

AssemblerX86::AssemblerX86(JitVM *vm)
    : vm_(vm)
{
}

auto AssemblerX86::beginCodeFragment() -> void
{
    vm_->beginCodeFragment();
}

auto AssemblerX86::endCodeFragment() -> void *
{
    return vm_->endCodeFragment();
}

auto AssemblerX86::encodeAndReg8Constant(X86Register8 reg, uint8_t constant)->void
{
    if (reg == AL) {
        vm_->addByte(0x24);
    }
    else {
        vm_->addByte(0x80);
        vm_->addByte(buildModRM(MOD_REG, 4, reg));
    }
    vm_->addByte(constant);
}

auto AssemblerX86::encodeCall(NativeAddress fn)->void
{
    auto addr = reinterpret_cast<uint32_t>(fn);
    vm_->addByte(0xE8);

    // The operand to CALL is a signed quantity relative to the end of
    // the instruction

    auto delta = fn - (vm_->nextByte() + sizeof(uint32_t));

    for (auto i = 0; i < 4; i++) {
        vm_->addByte(delta & 0xFF);
        delta >>= 8;
    }
}

auto AssemblerX86::encodeJumpIndirect(X86Register reg, uint32_t offset)->void
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

    uint8_t modrm = buildModRM(mod, static_cast<X86Register>(4), reg);

    vm_->addByte(0xFF);
    vm_->addByte(modrm);

    while (offsetBytes--) {
        vm_->addByte(offset & 0xFF);
        offset >>= 8;
    }
}

auto AssemblerX86::encodeLAHF()->void
{
    vm_->addByte(0x9F);
}


auto AssemblerX86::encodeMoveRegReg(X86Register dst, X86Register src)->void
{
    uint8_t modrm = buildModRM(MOD_REG, dst, src);

    vm_->addByte(0x8B);
    vm_->addByte(modrm);
}

auto AssemblerX86::encodeMoveRegReg8(X86Register8 dst, X86Register8 src)->void
{
    uint8_t modrm = buildModRM(MOD_REG, dst, src);

    vm_->addByte(0x8A);
    vm_->addByte(modrm);
}


auto AssemblerX86::encodeMoveRegPtrOffset(X86Register dst, X86Register ptr, uint32_t offset)->void
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

auto AssemblerX86::encodeMoveReg8PtrOffset(X86Register8 dst, X86Register ptr, uint32_t offset)->void
{
    MOD mod = MOD_INDIRECT;
    int offsetBytes = 0;

    assert(ptr != ESP);

    if (offset) {
        if (offset == 0 && ptr != EBP) {
            mod = MOD_INDIRECT;
        } else if (offset < 0x80 || offset > 0xFFFFFF80) {
            mod = MOD_DISP8;
            offsetBytes = 1;
        }
        else {
            mod = MOD_DISP32;
            offsetBytes = 4;
        }
    }

    uint8_t modrm = buildModRM(mod, dst, ptr);

    vm_->addByte(0x8A);
    vm_->addByte(modrm);

    while (offsetBytes--) {
        vm_->addByte(offset & 0xFF);
        offset >>= 8;
    }
}

auto AssemblerX86::encodeMoveRegConstant(X86Register dst, uint32_t c)->void
{
    vm_->addByte(0xB8 | dst);

    for (auto i = 0; i < 4; i++) {
        vm_->addByte(c & 0xFF);
        c >>= 8;
    }
}

auto AssemblerX86::encodeMoveReg8Constant(X86Register8 reg, uint8_t data)->void
{
    assert(reg >= 0 && reg < 8);
    vm_->addByte(0xB0 | reg);
    vm_->addByte(data);
}

auto AssemblerX86::encodeOrRegReg8(X86Register8 dst, X86Register8 src)->void
{
    vm_->addByte(0x08);
    vm_->addByte(buildModRM(MOD_REG, src, dst));
}

auto AssemblerX86::encodeXchgReg8(X86Register8 reg1, X86Register8 reg2)->void
{
    assert(reg1 >= 0 && reg1 < 8);
    assert(reg2 >= 0 && reg2 < 8);
    vm_->addByte(0x86);
    vm_->addByte(buildModRM(MOD_REG, reg1, reg2));
}

auto AssemblerX86::encodeXorReg(X86Register dst, X86Register src)->void
{
    assert(src >= 0 && src < 8);
    assert(dst >= 0 && dst < 8);
    vm_->addByte(0x31);
    vm_->addByte(buildModRM(MOD_REG, dst, src));
}

auto AssemblerX86::encodePopRegister(X86Register reg)->void
{
    vm_->addByte(0x58 | reg);
}

auto AssemblerX86::encodePushRegister(X86Register reg)->void
{
    vm_->addByte(0x50 | reg);
}

auto AssemblerX86::encodeRet() -> void
{
    vm_->addByte(0xC3);
}

auto AssemblerX86::encodeShiftRightReg(X86Register reg, uint8_t shift)->void
{
    if (shift == 1) {
        vm_->addByte(0xD1);
        vm_->addByte(buildModRM(MOD_REG, 5, reg));
    }
    else {
        vm_->addByte(0xC1);
        vm_->addByte(buildModRM(MOD_REG, 5, reg));
        vm_->addByte(shift);
    }
}


auto AssemblerX86::buildModRM(MOD mod, unsigned reg, unsigned mem)->uint8_t
{
    assert(reg >= 0 && reg < 8);
    assert(mem >= 0 && mem < 8);

    return mod | (reg << 3) | mem;
}

