#include <gtest/gtest.h>
#include "../vm/system.h"
#include "../render/visor.h"

class GraphicsSuite : public ::testing::Test {
protected:
    sigil::vmnode_t *vmroot;
    sigil::status_t status;

    void SetUp() override {
        vmroot = sigil::system::spawn_fakeroot(0, nullptr);
        status = sigil::visor::initialize(vmroot);
    }

    void TearDown() override {
        if (vmroot) vmroot->deinit();
    }
};

/*
    TODO: Add tests for
    Window creation/teardown
    Vulkan initialization
    Visor initialization
*/
TEST_F(GraphicsSuite, visor_cleanup) {
    ASSERT_EQ(sigil::system::shutdown(), sigil::VM_OK);
    vmroot = nullptr;  // Prevent double deinit in TearDown
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}