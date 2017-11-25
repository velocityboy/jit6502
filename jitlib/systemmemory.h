#pragma once

#include <stdint.h>
#include <array>
#include <string>
#include <unordered_map>

#include "types.h"

//
// SystemMemory represents the memory of the running 6502 system. It contains
// - any loaded ROM images, to which virtual writes will be ignored
// - device I/O regions
// - installed RAM
//
class SystemMemory
{
public:
    enum { SIZE = 65536 };
    enum { PAGE_SIZE = 256 };
    enum { PAGES = SIZE / PAGE_SIZE };

    using PageIndex = uint16_t;
    using PageOffset = uint16_t;

    static_assert(SIZE % PAGE_SIZE == 0, "Pages must evenly divide memory size");

    class IOHandler
    {
    public:
        virtual ~IOHandler();
        virtual auto read(Address addr)->uint8_t = 0;
        virtual auto write(Address addr, uint8_t data)->void = 0;
    };



    // Mixed means requires special handling. It can mean RAM and ROM, any partially filled
    // page (so configuration should try to avoid those), or any page that includes IO
    // handlers.
    enum PageFlags {
        EmptyFlag = 0x00,
        WriteableFlag = 0x01,
        ReadableFlag = 0x02,
        ReadWriteableFlag = 0x03,
        MixedFlag = 0x04, 
    };

    enum PageType {
        Empty,
        RAM,
        ROM,
        IO,
        Mixed
    };

    SystemMemory();

    // setup
    auto installROM(Address baseAddress, const std::vector<uint8_t> &contents)->void;
    auto installRAM(Address baseAddress, AddressSize length)->void;
    auto installIO(Address baseAddress, AddressSize length, IOHandler *handler)->void;

private:
    using Memory = std::array<uint8_t, SIZE>;
    using PageFlagsArray = std::array<PageFlags, PAGES>;
    using MixedPageHandlers = std::array<IOHandler *, PAGE_SIZE>;
    using MixedPageHandlerMap = std::unordered_map<PageIndex, MixedPageHandlers>;

    auto pageOf(Address address)->PageIndex;
    auto pageOffsetOf(Address address)->PageOffset;
    auto pageTypeToFlags(PageType type)->PageFlags;
    auto pageTypeToString(PageType type)->std::string;

    auto installRange(Address baseAddress, size_t length, PageType type, IOHandler *handler)->void;
    auto installPage(PageIndex page, PageOffset startOffset, PageOffset endOffset, PageType type, IOHandler *handler)->bool;

    class ROMHandler : public IOHandler {
    public:
        ROMHandler(const Memory &memory);
        virtual auto read(Address addr)->uint8_t override;
        virtual auto write(Address addr, uint8_t data)->void override;
    private:
        const Memory &memory_;
    };

    class RAMHandler : public IOHandler {
    public:
        RAMHandler(Memory &memory);
        virtual auto read(Address addr)->uint8_t override;
        virtual auto write(Address addr, uint8_t data)->void override;
    private:
        Memory &memory_;
    };

    Memory memory_;
    PageFlagsArray pageFlags_;
    MixedPageHandlerMap mixedPageHandlerMap_;
    ROMHandler romHandler_;
    RAMHandler ramHandler_;
};
