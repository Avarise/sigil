#include <gtest/gtest.h>
#include "../vm/system.h"

class ParserSuite : public ::testing::Test {
protected:
    sigil::vmnode_t *vmroot;

    void SetUp() override {
        vmroot = sigil::system::spawn_fakeroot(0, nullptr);
    }

    void TearDown() override {
        if (vmroot) vmroot->deinit();
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