#include <gtest/gtest.h>
#include "virtual-machine.h"

class SecuritySuite : public ::testing::Test {
    protected:
    void SetUp() override {
        sigil::virtual_machine::initialize(0, nullptr);
    }

    void TearDown() override {
        sigil::virtual_machine::deinitialize();
    }
};

TEST_F(SecuritySuite, xor_encode_mechanism) {
    ASSERT_EQ(sigil::virtual_machine::deinitialize(), sigil::VM_OK);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}