#include <gtest/gtest.h>
#include "station.h"
#include "virtual-machine.h"
#include "utils.h"

class InitializationSuite : public ::testing::Test {
protected:
    void SetUp() override {
        //sigil::status_t status = sigil::virtual_machine::initialize(0, nullptr);
    }

    void TearDown() override {
        //sigil::virtual_machine::deinitialize();
    }
};

TEST_F(InitializationSuite, vm_startup_test) {
    ASSERT_EQ(sigil::virtual_machine::initialize(0, nullptr), sigil::VM_OK);
    ASSERT_EQ(sigil::virtual_machine::get_state(), sigil::VM_OK);
}



TEST_F(InitializationSuite, vm_shutdown_test) {
    ASSERT_EQ(sigil::virtual_machine::deinitialize(), sigil::VM_OK);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}