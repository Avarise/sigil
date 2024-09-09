#include <gtest/gtest.h>
#include "../vm/system.h"

class SecuritySuite : public ::testing::Test {
protected:
    sigil::vmnode_t *vmroot;

    void SetUp() override {
        vmroot = sigil::system::spawn_fakeroot(0, nullptr);
    }

    void TearDown() override {
        if (vmroot) vmroot->deinit();
    }
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}