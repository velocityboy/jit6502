#include "stdafx.h"

#include "exceptions.h"
#include "systemmemory.h"

#include <algorithm>
#include <assert.h>
#include <iomanip>
#include <iterator>
#include <sstream>

using std::begin;
using std::copy;
using std::end;
using std::fill;
using std::find_if;
using std::hex;
using std::runtime_error;
using std::setfill;
using std::setw;
using std::string;

using oss = std::ostringstream;

SystemMemory::IOHandler::~IOHandler()
{
}

SystemMemory::SystemMemory()
    : romHandler_(memory_)
    , ramHandler_(memory_)
{
    fill(begin(pageFlags_), end(pageFlags_), EmptyFlag);
}

auto SystemMemory::installROM(Address baseAddress, const std::vector<uint8_t> &contents)->void
{
    installRange(baseAddress, contents.size(), ROM, &romHandler_);
    copy(begin(contents), end(contents), begin(memory_) + baseAddress);
}

auto SystemMemory::installRAM(Address baseAddress, AddressSize length)->void
{
    installRange(baseAddress, length, ROM, &ramHandler_);
}

auto SystemMemory::installIO(Address baseAddress, AddressSize length, IOHandler *handler)->void
{
    installRange(baseAddress, length, IO, handler);
}

auto SystemMemory::readByte(Address address)->uint8_t
{
    auto page = pageOf(address);
    auto flags = pageFlags_[page];

    if ((flags & ReadableFlag) != 0) {
        return memory_[address];
    }

    if ((flags & MixedFlag) != 0) {
        auto handlers = mixedPageHandlerMap_.find(page);
        if (handlers != end(mixedPageHandlerMap_)) {
            auto handler = handlers->second[pageOffsetOf(address)];
            if (handler != nullptr) {
                return handler->read(address);
            }
        }
    }

    // $TODO nothing at address, should really throw an exception and handle
    return 0xFF;
}

auto SystemMemory::readWord(Address address)->uint16_t
{
    auto low = readByte(address);
    auto high = readByte(address + 1);
    return (high << 8) | low;
}

auto SystemMemory::pageOf(Address address)->PageIndex
{
    return address / PAGE_SIZE;
}

auto SystemMemory::pageOffsetOf(Address address)->PageOffset
{
    return address % PAGE_SIZE;
}

auto SystemMemory::pageTypeToFlags(PageType type)->PageFlags
{
    switch (type) {
    case Empty:
        return EmptyFlag;

    case RAM:
        return ReadWriteableFlag;

    case ROM:
        return ReadableFlag;

    case IO:
    case Mixed:
        return MixedFlag;
    }

    assert(false);
    return EmptyFlag;
}

auto SystemMemory::pageTypeToString(PageType type)->string
{
    switch (type) {
    case Empty:
        return "Empty";

    case RAM:
        return "RAM";

    case ROM:
        return "ROM";

    case IO:
        return "IO";

    case Mixed:
        return "Mixed";
    }

    return "Unknown";
}

auto SystemMemory::installRange(Address baseAddress, size_t length, PageType type, IOHandler *handler)->void
{
    if (length == 0) {
        oss()
            << pageTypeToString(type)
            << " at address "
            << setw(4) << hex << setfill('0') << baseAddress
            << " is empty."
            << throwError;
    }

    if (length > SIZE) {
        oss()
            << pageTypeToString(type)
            << " at address "
            << setw(4) << hex << setfill('0') << baseAddress
            << " is too large for memory."
            << throwError;
    }

    if (baseAddress + length > SIZE) {
        oss()
            << pageTypeToString(type)
            << " at address "
            << setw(4) << hex << setfill('0') << baseAddress
            << " runs off end of memory."
            << throwError;
    }

    auto startPage = pageOf(baseAddress);
    auto endPage = pageOf(baseAddress + static_cast<AddressSize>(length));
    auto startOffset = pageOffsetOf(baseAddress);
    auto endOffset = pageOffsetOf(baseAddress + static_cast<AddressSize>(length));

    if (endPage == 0) {
        endPage = PAGES;
    }

    auto overlaps = false;

    while (startPage < endPage && !overlaps) {
        overlaps = !installPage(startPage, startOffset, PAGE_SIZE, ROM, &romHandler_);
        startPage++;
        startOffset = 0;
    }

    if (!overlaps) {
        assert(startPage == endPage);
        overlaps = !installPage(startPage, startOffset, endOffset, ROM, &romHandler_);
    }

    if (overlaps) {
        oss()
            << "ROM at address "
            << setw(4) << hex << setfill('0') << baseAddress
            << " overlaps already installed virtual hardware."
            << throwError;
    }
}

auto SystemMemory::installPage(PageIndex page, PageOffset startOffset, PageOffset endOffset, PageType type, IOHandler *handler)->bool
{
    if (startOffset == endOffset) {
        // empty region
        return true;
    }

    // Best case: the range to install covers the entire page and the page is currently empty.
    if (startOffset == 0 && endOffset == PAGE_SIZE && pageFlags_[page] == EmptyFlag) {
        pageFlags_[page] = pageTypeToFlags(type);
        return true;
    }

    // Either we're not talking about a full page, or the page is already another type. 
    // If it's not mixed type, then it's already fully allocated to some other type and
    // we have an overlap.
    auto overlaps = pageFlags_[page] != EmptyFlag && (pageFlags_[page] & MixedFlag) == 0;

    if (!overlaps) {
        pageFlags_[page] = MixedFlag;

        auto &pageHandlers = mixedPageHandlerMap_[page];
        auto rangeStart = begin(pageHandlers) + startOffset;
        auto rangeEnd = begin(pageHandlers) + endOffset;

        auto usedEntry = find_if(rangeStart, rangeEnd, [](auto entry) {
            return entry != nullptr;
        });
       
        overlaps = usedEntry != begin(pageHandlers) + endOffset;
        if (!overlaps) {
            fill(rangeStart, rangeEnd, handler);
        }
    }

    return !overlaps;
}


// Handlers for RAM and ROM. These handlers are only used when a page is mixed use --
// the overhead is not incurred on pages which only contain one type of memory.
//
SystemMemory::ROMHandler::ROMHandler(const Memory &memory)
    : memory_(memory)
{
}

auto SystemMemory::ROMHandler::read(Address addr)->uint8_t
{
    return memory_[addr];
}

auto SystemMemory::ROMHandler::write(Address addr, uint8_t data)->void
{
    //$TODO might be nice to log this - could be bug in hosted code
}

SystemMemory::RAMHandler::RAMHandler(Memory &memory)
    : memory_(memory)
{
}

auto SystemMemory::RAMHandler::read(Address addr)->uint8_t
{
    return memory_[addr];
}

auto SystemMemory::RAMHandler::write(Address addr, uint8_t data)->void
{
    memory_[addr] = data;
}

