#include <gtest/gtest.h>
#include "virtual-machine.h"
#include "system.h"

class ParserSuite : public ::testing::Test {
    protected:

    void SetUp() override {
        sigil::virtual_machine::initialize(0,nullptr);
    }

    void TearDown() override {
        sigil::virtual_machine::deinitialize();
    }
};

/*
    Add tests for parser:
    Processing basic commands
    Running simple scripts
*/

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}