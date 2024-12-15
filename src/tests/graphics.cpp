#include <gtest/gtest.h>
#include "virtual-machine.h"
#include "system.h"
#include "visor.h"

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
TEST_F(GraphicsSuite, visor_cleanup) {

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}