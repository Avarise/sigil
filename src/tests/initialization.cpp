#include <gtest/gtest.h>
#include <cstddef>
#include "../vm/system.h"
#include "../vm/core.h"

class InitializationSuite : public ::testing::Test {
protected:
    sigil::vmnode_t *vmroot;

    void SetUp() override {
        vmroot = sigil::system::spawn_fakeroot(0, nullptr);
    }

    void TearDown() override {
        if (vmroot) vmroot->deinit();
    }
};

TEST_F(InitializationSuite, vm_startup_test) {
    ASSERT_NE(vmroot, nullptr);
    EXPECT_EQ((vmroot->name.value.compare(VM_NODE_VMROOT)), 0);  // Assuming default name is empty string
    EXPECT_TRUE(vmroot->subnodes.size() == (size_t)2);
}

TEST_F(InitializationSuite, vm_shutdown_test) {
    ASSERT_EQ(sigil::system::shutdown(), sigil::VM_OK);
    vmroot = nullptr;  // Prevent double deinit in TearDown
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}