#include <gtest/gtest.h>
#include "virtual-machine.h"
#include "system.h"

class NetworkingSuite : public ::testing::Test {
    protected:
    void SetUp() override {
        sigil::virtual_machine::initialize(0, nullptr);
    }

    void TearDown() override {
        sigil::virtual_machine::deinitialize();
    }
};

/*
    TODO: Add tests for
    Station initialization/teardown
    Server creation/teardown
    Get a file from server
    Send a command to a server
*/

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}