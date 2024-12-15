#include <gtest/gtest.h>
#include "virtual-machine.h"
#include "system.h"
#include "visor.h"
#include "vulkan.h"

class GraphicsSuite : public ::testing::Test {
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
    Window creation/teardown
    Vulkan initialization
    Visor initialization
*/
TEST_F(GraphicsSuite, vulkan_startup) {
    ASSERT_EQ(sigil::vulkan::initialize(), sigil::VM_OK);
}

TEST_F(GraphicsSuite, visor_startup) {
    ASSERT_EQ(sigil::visor::initialize(), sigil::VM_OK);
}

TEST_F(GraphicsSuite, window_creation) {

}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}