#include "stdafx.h"
#include "CppUnitTest.h"
#include "../jitlib/systemmemory.h"

#include <stdexcept>
#include <vector>

using std::runtime_error;
using std::vector;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace jittests
{		
	TEST_CLASS(SystemMemoryTest)
	{
	public:
		
		TEST_METHOD(TestSimpleInstall)
		{
            SystemMemory memory;
            vector<uint8_t> ROM;
            ROM.resize(512);

            // this should not throw an exception.
            memory.installROM(0xA000, ROM);
		}

        TEST_METHOD(TestAdjacentInstalls)
        {
            SystemMemory memory;
            vector<uint8_t> ROM;
            ROM.resize(512);

            // this should not throw an exception.
            memory.installROM(0xA000, ROM);
            memory.installRAM(0xA200, 512);
        }

        TEST_METHOD(TestOverlappingInstalls)
        {
            SystemMemory memory;
            vector<uint8_t> ROM;
            ROM.resize(512);

            memory.installROM(0xA000, ROM);
            // this should throw an exception.

            auto threw = false;
            try {
                memory.installRAM(0xA100, 512);
            }
            catch (runtime_error) {
                threw = true;
            }

            Assert::IsTrue(threw, L"Overlapping install ranges should throw exception");
        }

    };
}
