#include <gtest/gtest.h>
#include "utils.h"

class LibrarySuite : public ::testing::Test {
protected:
    void SetUp() override {
        //sigil::status_t status = sigil::virtual_machine::initialize(0, nullptr);
    }

    void TearDown() override {
        //sigil::virtual_machine::deinitialize();
    }
};

TEST_F(LibrarySuite, GeneratesValidNumbersInRange) {
    constexpr uint32_t kMin = 10;
    constexpr uint32_t kMax = 100;
    constexpr size_t kIterations = 1024 * 1024;
    
    for (size_t i = 0; i < kIterations; ++i) {
        uint32_t result = sigil::random_u32_scoped(kMin, kMax);
        EXPECT_GE(result, kMin);
        EXPECT_LE(result, kMax);
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

