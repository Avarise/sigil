#include <gtest/gtest.h>
#include "system.h"
#include "virtual-machine.h"

class PerformanceSuite : public ::testing::Test {
    protected:

    void SetUp() override {
        sigil::virtual_machine::initialize(0,nullptr);
    }

    void TearDown() override {
        sigil::virtual_machine::deinitialize();
    }
};

/*
    Add tests for performance:
    Hitting FPS targets
    
*/

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}