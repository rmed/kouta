#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/utils/enum-set.hpp>

namespace kouta::tests::utils
{
    using namespace kouta::utils;

    enum class TestEnum : std::size_t
    {
        A,
        B,
        C,
        D,

        _Total
    };

    /// @brief Test the behaviour of the EnumSet when it is default-constructed.
    TEST(UtilsTest, EnumSetEmpty)
    {
        EnumSet<TestEnum> set{};

        EXPECT_EQ(set.size(), 4);
        EXPECT_EQ(set.count(), 0);

        EXPECT_EQ(set[TestEnum::A], false);
        EXPECT_EQ(set[TestEnum::B], false);
        EXPECT_EQ(set[TestEnum::C], false);
        EXPECT_EQ(set[TestEnum::D], false);

        EXPECT_EQ(set.test(TestEnum::A), false);
        EXPECT_EQ(set.test(TestEnum::B), false);
        EXPECT_EQ(set.test(TestEnum::C), false);
        EXPECT_EQ(set.test(TestEnum::D), false);

        set.set(TestEnum::C);

        EXPECT_EQ(set[TestEnum::C], true);
        EXPECT_EQ(set.test(TestEnum::C), true);

        set[TestEnum::C] = false;

        EXPECT_EQ(set[TestEnum::C], false);
        EXPECT_EQ(set.test(TestEnum::C), false);
    }

    /// @brief Test the behaviour of the EnumSet when it is constructed with initial values.
    TEST(UtilsTest, EnumSetInitialValue)
    {
        EnumSet<TestEnum> set{TestEnum::B, TestEnum::D};

        EXPECT_EQ(set.size(), 4);
        EXPECT_EQ(set.count(), 2);

        EXPECT_EQ(set[TestEnum::A], false);
        EXPECT_EQ(set[TestEnum::B], true);
        EXPECT_EQ(set[TestEnum::C], false);
        EXPECT_EQ(set[TestEnum::D], true);

        EXPECT_EQ(set.test(TestEnum::A), false);
        EXPECT_EQ(set.test(TestEnum::B), true);
        EXPECT_EQ(set.test(TestEnum::C), false);
        EXPECT_EQ(set.test(TestEnum::D), true);

        set.set(TestEnum::C);

        EXPECT_EQ(set[TestEnum::C], true);
        EXPECT_EQ(set.test(TestEnum::C), true);

        set[TestEnum::C] = false;

        EXPECT_EQ(set[TestEnum::C], false);
        EXPECT_EQ(set.test(TestEnum::C), false);
    }
}  // namespace kouta::tests::utils
