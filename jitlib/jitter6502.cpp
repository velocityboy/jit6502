#include "stdafx.h"

#include "jitter6502.h"

#include "exceptions.h"
#include "jitvm.h"
#include "assembler_x86.h"
#include "systemmemory.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using std::array;
using std::endl;
using std::hex;
using std::runtime_error;
using std::setfill;
using std::setw;

using oss = std::ostringstream;

Jitter6502::Jitter6502(JitVM *vm, AssemblerX86 *assembler, SystemMemory *memory)
    : vm_(vm)
    , assembler_(assembler)
    , memory_(memory)
{
    buildReentryStub();
}

auto Jitter6502::boot()->void
{
    const TargetAddress RESET = 0xFFFC;

    auto ip = memory_->readWord(RESET);
    auto start = jit(ip);

    try {
        entryStub_(vm_, start);
    }
    catch (runtime_error err) {
        auto errText = oss{};
        errText << "Runtime threw exception: " << err.what() << endl;
        OutputDebugStringA(errText.str().c_str());
    }
}

auto Jitter6502::jit(TargetAddress ip)->NativeAddress
{
    auto inBlock = true;

    vm_->beginCodeFragment();
    while (true) {
        auto byte = memory_->readByte(ip++);
        if (!(this->*jitters_[byte])(&ip)) {
            break;
        }
    }
    return vm_->endCodeFragment();
}

auto Jitter6502::buildReentryStub()->void
{
    // Set up stack, EDI -> VM, and jump to entry
    assembler_->beginCodeFragment();
    assembler_->encodePushRegister(EBP);
    assembler_->encodeMoveRegReg(EBP, ESP);
    assembler_->encodeMoveRegPtrOffset(EDI, EBP, 8);
    assembler_->encodeJumpIndirect(EBP, 12);
    entryStub_ = static_cast<Entry>(assembler_->endCodeFragment());

    // Set up return
    assembler_->beginCodeFragment();
    assembler_->encodeMoveRegReg(ESP, EBP);
    assembler_->encodePopRegister(EBP);
    assembler_->encodeRet();
    exitStub_ = static_cast<Exit>(assembler_->endCodeFragment());
}

auto Jitter6502::invalidOpcodeStub(TargetAddress addr)->void 
{
    oss()
        << "Execution terminated at "
        << setw(4) << setfill('0') << hex << addr
        << "; invalid opcode"
        << throwError;
}

auto Jitter6502::jitInvalidOpcode(TargetAddress *ip)->bool
{
    assembler_->encodeMoveRegConstant(EAX, *ip - 1);
    assembler_->encodePushRegister(EAX);
    assembler_->encodeCall(&invalidOpcodeStub);
    // no need to clean up, exception will have been thrown
    return false;
}

array<Jitter6502::InstructionJitter, 256> Jitter6502::jitters_ = {
    /*00*/ &Jitter6502::jitInvalidOpcode,
    /*01*/ &Jitter6502::jitInvalidOpcode,
    /*02*/ &Jitter6502::jitInvalidOpcode,
    /*03*/ &Jitter6502::jitInvalidOpcode,
    /*04*/ &Jitter6502::jitInvalidOpcode,
    /*05*/ &Jitter6502::jitInvalidOpcode,
    /*06*/ &Jitter6502::jitInvalidOpcode,
    /*07*/ &Jitter6502::jitInvalidOpcode,
    /*08*/ &Jitter6502::jitInvalidOpcode,
    /*09*/ &Jitter6502::jitInvalidOpcode,
    /*0A*/ &Jitter6502::jitInvalidOpcode,
    /*0B*/ &Jitter6502::jitInvalidOpcode,
    /*0C*/ &Jitter6502::jitInvalidOpcode,
    /*0D*/ &Jitter6502::jitInvalidOpcode,
    /*0E*/ &Jitter6502::jitInvalidOpcode,
    /*0F*/ &Jitter6502::jitInvalidOpcode,

    /*10*/ &Jitter6502::jitInvalidOpcode,
    /*11*/ &Jitter6502::jitInvalidOpcode,
    /*12*/ &Jitter6502::jitInvalidOpcode,
    /*13*/ &Jitter6502::jitInvalidOpcode,
    /*14*/ &Jitter6502::jitInvalidOpcode,
    /*15*/ &Jitter6502::jitInvalidOpcode,
    /*16*/ &Jitter6502::jitInvalidOpcode,
    /*17*/ &Jitter6502::jitInvalidOpcode,
    /*18*/ &Jitter6502::jitInvalidOpcode,
    /*19*/ &Jitter6502::jitInvalidOpcode,
    /*1A*/ &Jitter6502::jitInvalidOpcode,
    /*1B*/ &Jitter6502::jitInvalidOpcode,
    /*1C*/ &Jitter6502::jitInvalidOpcode,
    /*1D*/ &Jitter6502::jitInvalidOpcode,
    /*1E*/ &Jitter6502::jitInvalidOpcode,
    /*1F*/ &Jitter6502::jitInvalidOpcode,

    /*20*/ &Jitter6502::jitInvalidOpcode,
    /*21*/ &Jitter6502::jitInvalidOpcode,
    /*22*/ &Jitter6502::jitInvalidOpcode,
    /*23*/ &Jitter6502::jitInvalidOpcode,
    /*24*/ &Jitter6502::jitInvalidOpcode,
    /*25*/ &Jitter6502::jitInvalidOpcode,
    /*26*/ &Jitter6502::jitInvalidOpcode,
    /*27*/ &Jitter6502::jitInvalidOpcode,
    /*28*/ &Jitter6502::jitInvalidOpcode,
    /*29*/ &Jitter6502::jitInvalidOpcode,
    /*2A*/ &Jitter6502::jitInvalidOpcode,
    /*2B*/ &Jitter6502::jitInvalidOpcode,
    /*2C*/ &Jitter6502::jitInvalidOpcode,
    /*2D*/ &Jitter6502::jitInvalidOpcode,
    /*2E*/ &Jitter6502::jitInvalidOpcode,
    /*2F*/ &Jitter6502::jitInvalidOpcode,

    /*30*/ &Jitter6502::jitInvalidOpcode,
    /*31*/ &Jitter6502::jitInvalidOpcode,
    /*32*/ &Jitter6502::jitInvalidOpcode,
    /*33*/ &Jitter6502::jitInvalidOpcode,
    /*34*/ &Jitter6502::jitInvalidOpcode,
    /*35*/ &Jitter6502::jitInvalidOpcode,
    /*36*/ &Jitter6502::jitInvalidOpcode,
    /*37*/ &Jitter6502::jitInvalidOpcode,
    /*38*/ &Jitter6502::jitInvalidOpcode,
    /*39*/ &Jitter6502::jitInvalidOpcode,
    /*3A*/ &Jitter6502::jitInvalidOpcode,
    /*3B*/ &Jitter6502::jitInvalidOpcode,
    /*3C*/ &Jitter6502::jitInvalidOpcode,
    /*3D*/ &Jitter6502::jitInvalidOpcode,
    /*3E*/ &Jitter6502::jitInvalidOpcode,
    /*3F*/ &Jitter6502::jitInvalidOpcode,

    /*40*/ &Jitter6502::jitInvalidOpcode,
    /*41*/ &Jitter6502::jitInvalidOpcode,
    /*42*/ &Jitter6502::jitInvalidOpcode,
    /*43*/ &Jitter6502::jitInvalidOpcode,
    /*44*/ &Jitter6502::jitInvalidOpcode,
    /*45*/ &Jitter6502::jitInvalidOpcode,
    /*46*/ &Jitter6502::jitInvalidOpcode,
    /*47*/ &Jitter6502::jitInvalidOpcode,
    /*48*/ &Jitter6502::jitInvalidOpcode,
    /*49*/ &Jitter6502::jitInvalidOpcode,
    /*4A*/ &Jitter6502::jitInvalidOpcode,
    /*4B*/ &Jitter6502::jitInvalidOpcode,
    /*4C*/ &Jitter6502::jitInvalidOpcode,
    /*4D*/ &Jitter6502::jitInvalidOpcode,
    /*4E*/ &Jitter6502::jitInvalidOpcode,
    /*4F*/ &Jitter6502::jitInvalidOpcode,

    /*50*/ &Jitter6502::jitInvalidOpcode,
    /*51*/ &Jitter6502::jitInvalidOpcode,
    /*52*/ &Jitter6502::jitInvalidOpcode,
    /*53*/ &Jitter6502::jitInvalidOpcode,
    /*54*/ &Jitter6502::jitInvalidOpcode,
    /*55*/ &Jitter6502::jitInvalidOpcode,
    /*56*/ &Jitter6502::jitInvalidOpcode,
    /*57*/ &Jitter6502::jitInvalidOpcode,
    /*58*/ &Jitter6502::jitInvalidOpcode,
    /*59*/ &Jitter6502::jitInvalidOpcode,
    /*5A*/ &Jitter6502::jitInvalidOpcode,
    /*5B*/ &Jitter6502::jitInvalidOpcode,
    /*5C*/ &Jitter6502::jitInvalidOpcode,
    /*5D*/ &Jitter6502::jitInvalidOpcode,
    /*5E*/ &Jitter6502::jitInvalidOpcode,
    /*5F*/ &Jitter6502::jitInvalidOpcode,

    /*60*/ &Jitter6502::jitInvalidOpcode,
    /*61*/ &Jitter6502::jitInvalidOpcode,
    /*62*/ &Jitter6502::jitInvalidOpcode,
    /*63*/ &Jitter6502::jitInvalidOpcode,
    /*64*/ &Jitter6502::jitInvalidOpcode,
    /*65*/ &Jitter6502::jitInvalidOpcode,
    /*66*/ &Jitter6502::jitInvalidOpcode,
    /*67*/ &Jitter6502::jitInvalidOpcode,
    /*68*/ &Jitter6502::jitInvalidOpcode,
    /*69*/ &Jitter6502::jitInvalidOpcode,
    /*6A*/ &Jitter6502::jitInvalidOpcode,
    /*6B*/ &Jitter6502::jitInvalidOpcode,
    /*6C*/ &Jitter6502::jitInvalidOpcode,
    /*6D*/ &Jitter6502::jitInvalidOpcode,
    /*6E*/ &Jitter6502::jitInvalidOpcode,
    /*6F*/ &Jitter6502::jitInvalidOpcode,

    /*70*/ &Jitter6502::jitInvalidOpcode,
    /*71*/ &Jitter6502::jitInvalidOpcode,
    /*72*/ &Jitter6502::jitInvalidOpcode,
    /*73*/ &Jitter6502::jitInvalidOpcode,
    /*74*/ &Jitter6502::jitInvalidOpcode,
    /*75*/ &Jitter6502::jitInvalidOpcode,
    /*76*/ &Jitter6502::jitInvalidOpcode,
    /*77*/ &Jitter6502::jitInvalidOpcode,
    /*78*/ &Jitter6502::jitInvalidOpcode,
    /*79*/ &Jitter6502::jitInvalidOpcode,
    /*7A*/ &Jitter6502::jitInvalidOpcode,
    /*7B*/ &Jitter6502::jitInvalidOpcode,
    /*7C*/ &Jitter6502::jitInvalidOpcode,
    /*7D*/ &Jitter6502::jitInvalidOpcode,
    /*7E*/ &Jitter6502::jitInvalidOpcode,
    /*7F*/ &Jitter6502::jitInvalidOpcode,

    /*80*/ &Jitter6502::jitInvalidOpcode,
    /*81*/ &Jitter6502::jitInvalidOpcode,
    /*82*/ &Jitter6502::jitInvalidOpcode,
    /*83*/ &Jitter6502::jitInvalidOpcode,
    /*84*/ &Jitter6502::jitInvalidOpcode,
    /*85*/ &Jitter6502::jitInvalidOpcode,
    /*86*/ &Jitter6502::jitInvalidOpcode,
    /*87*/ &Jitter6502::jitInvalidOpcode,
    /*88*/ &Jitter6502::jitInvalidOpcode,
    /*89*/ &Jitter6502::jitInvalidOpcode,
    /*8A*/ &Jitter6502::jitInvalidOpcode,
    /*8B*/ &Jitter6502::jitInvalidOpcode,
    /*8C*/ &Jitter6502::jitInvalidOpcode,
    /*8D*/ &Jitter6502::jitInvalidOpcode,
    /*8E*/ &Jitter6502::jitInvalidOpcode,
    /*8F*/ &Jitter6502::jitInvalidOpcode,

    /*90*/ &Jitter6502::jitInvalidOpcode,
    /*91*/ &Jitter6502::jitInvalidOpcode,
    /*92*/ &Jitter6502::jitInvalidOpcode,
    /*93*/ &Jitter6502::jitInvalidOpcode,
    /*94*/ &Jitter6502::jitInvalidOpcode,
    /*95*/ &Jitter6502::jitInvalidOpcode,
    /*96*/ &Jitter6502::jitInvalidOpcode,
    /*97*/ &Jitter6502::jitInvalidOpcode,
    /*98*/ &Jitter6502::jitInvalidOpcode,
    /*99*/ &Jitter6502::jitInvalidOpcode,
    /*9A*/ &Jitter6502::jitInvalidOpcode,
    /*9B*/ &Jitter6502::jitInvalidOpcode,
    /*9C*/ &Jitter6502::jitInvalidOpcode,
    /*9D*/ &Jitter6502::jitInvalidOpcode,
    /*9E*/ &Jitter6502::jitInvalidOpcode,
    /*9F*/ &Jitter6502::jitInvalidOpcode,

    /*A0*/ &Jitter6502::jitInvalidOpcode,
    /*A1*/ &Jitter6502::jitInvalidOpcode,
    /*A2*/ &Jitter6502::jitInvalidOpcode,
    /*A3*/ &Jitter6502::jitInvalidOpcode,
    /*A4*/ &Jitter6502::jitInvalidOpcode,
    /*A5*/ &Jitter6502::jitInvalidOpcode,
    /*A6*/ &Jitter6502::jitInvalidOpcode,
    /*A7*/ &Jitter6502::jitInvalidOpcode,
    /*A8*/ &Jitter6502::jitInvalidOpcode,
    /*A9*/ &Jitter6502::jitInvalidOpcode,
    /*AA*/ &Jitter6502::jitInvalidOpcode,
    /*AB*/ &Jitter6502::jitInvalidOpcode,
    /*AC*/ &Jitter6502::jitInvalidOpcode,
    /*AD*/ &Jitter6502::jitInvalidOpcode,
    /*AE*/ &Jitter6502::jitInvalidOpcode,
    /*AF*/ &Jitter6502::jitInvalidOpcode,

    /*B0*/ &Jitter6502::jitInvalidOpcode,
    /*B1*/ &Jitter6502::jitInvalidOpcode,
    /*B2*/ &Jitter6502::jitInvalidOpcode,
    /*B3*/ &Jitter6502::jitInvalidOpcode,
    /*B4*/ &Jitter6502::jitInvalidOpcode,
    /*B5*/ &Jitter6502::jitInvalidOpcode,
    /*B6*/ &Jitter6502::jitInvalidOpcode,
    /*B7*/ &Jitter6502::jitInvalidOpcode,
    /*B8*/ &Jitter6502::jitInvalidOpcode,
    /*B9*/ &Jitter6502::jitInvalidOpcode,
    /*BA*/ &Jitter6502::jitInvalidOpcode,
    /*BB*/ &Jitter6502::jitInvalidOpcode,
    /*BC*/ &Jitter6502::jitInvalidOpcode,
    /*BD*/ &Jitter6502::jitInvalidOpcode,
    /*BE*/ &Jitter6502::jitInvalidOpcode,
    /*BF*/ &Jitter6502::jitInvalidOpcode,

    /*C0*/ &Jitter6502::jitInvalidOpcode,
    /*C1*/ &Jitter6502::jitInvalidOpcode,
    /*C2*/ &Jitter6502::jitInvalidOpcode,
    /*C3*/ &Jitter6502::jitInvalidOpcode,
    /*C4*/ &Jitter6502::jitInvalidOpcode,
    /*C5*/ &Jitter6502::jitInvalidOpcode,
    /*C6*/ &Jitter6502::jitInvalidOpcode,
    /*C7*/ &Jitter6502::jitInvalidOpcode,
    /*C8*/ &Jitter6502::jitInvalidOpcode,
    /*C9*/ &Jitter6502::jitInvalidOpcode,
    /*CA*/ &Jitter6502::jitInvalidOpcode,
    /*CB*/ &Jitter6502::jitInvalidOpcode,
    /*CC*/ &Jitter6502::jitInvalidOpcode,
    /*CD*/ &Jitter6502::jitInvalidOpcode,
    /*CE*/ &Jitter6502::jitInvalidOpcode,
    /*CF*/ &Jitter6502::jitInvalidOpcode,

    /*D0*/ &Jitter6502::jitInvalidOpcode,
    /*D1*/ &Jitter6502::jitInvalidOpcode,
    /*D2*/ &Jitter6502::jitInvalidOpcode,
    /*D3*/ &Jitter6502::jitInvalidOpcode,
    /*D4*/ &Jitter6502::jitInvalidOpcode,
    /*D5*/ &Jitter6502::jitInvalidOpcode,
    /*D6*/ &Jitter6502::jitInvalidOpcode,
    /*D7*/ &Jitter6502::jitInvalidOpcode,
    /*D8*/ &Jitter6502::jitInvalidOpcode,
    /*D9*/ &Jitter6502::jitInvalidOpcode,
    /*DA*/ &Jitter6502::jitInvalidOpcode,
    /*DB*/ &Jitter6502::jitInvalidOpcode,
    /*DC*/ &Jitter6502::jitInvalidOpcode,
    /*DD*/ &Jitter6502::jitInvalidOpcode,
    /*DE*/ &Jitter6502::jitInvalidOpcode,
    /*DF*/ &Jitter6502::jitInvalidOpcode,

    /*E0*/ &Jitter6502::jitInvalidOpcode,
    /*E1*/ &Jitter6502::jitInvalidOpcode,
    /*E2*/ &Jitter6502::jitInvalidOpcode,
    /*E3*/ &Jitter6502::jitInvalidOpcode,
    /*E4*/ &Jitter6502::jitInvalidOpcode,
    /*E5*/ &Jitter6502::jitInvalidOpcode,
    /*E6*/ &Jitter6502::jitInvalidOpcode,
    /*E7*/ &Jitter6502::jitInvalidOpcode,
    /*E8*/ &Jitter6502::jitInvalidOpcode,
    /*E9*/ &Jitter6502::jitInvalidOpcode,
    /*EA*/ &Jitter6502::jitInvalidOpcode,
    /*EB*/ &Jitter6502::jitInvalidOpcode,
    /*EC*/ &Jitter6502::jitInvalidOpcode,
    /*ED*/ &Jitter6502::jitInvalidOpcode,
    /*EE*/ &Jitter6502::jitInvalidOpcode,
    /*EF*/ &Jitter6502::jitInvalidOpcode,

    /*F0*/ &Jitter6502::jitInvalidOpcode,
    /*F1*/ &Jitter6502::jitInvalidOpcode,
    /*F2*/ &Jitter6502::jitInvalidOpcode,
    /*F3*/ &Jitter6502::jitInvalidOpcode,
    /*F4*/ &Jitter6502::jitInvalidOpcode,
    /*F5*/ &Jitter6502::jitInvalidOpcode,
    /*F6*/ &Jitter6502::jitInvalidOpcode,
    /*F7*/ &Jitter6502::jitInvalidOpcode,
    /*F8*/ &Jitter6502::jitInvalidOpcode,
    /*F9*/ &Jitter6502::jitInvalidOpcode,
    /*FA*/ &Jitter6502::jitInvalidOpcode,
    /*FB*/ &Jitter6502::jitInvalidOpcode,
    /*FC*/ &Jitter6502::jitInvalidOpcode,
    /*FD*/ &Jitter6502::jitInvalidOpcode,
    /*FE*/ &Jitter6502::jitInvalidOpcode,
    /*FF*/ &Jitter6502::jitInvalidOpcode,
};
