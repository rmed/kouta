#include <cstdint>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/io/parser.hpp>

namespace kouta::tests::io
{
    using namespace kouta::io;

    /// @brief Test the behaviour of the parser (Big Endian).
    ///
    /// @details
    /// The test succeeds if all values can be extracted from the buffer.
    TEST(IoTest, ParserBe)
    {
        std::vector<std::uint8_t> buf{
            // clang-format off
            // uint8: 254
            0xFE,
            // int8: -124
            0x84,
            // uin16: 7465
            0x1D, 0x29,
            // int16: -9827
            0xD9, 0x9D,
            // uint24: 1025
            0x00, 0x04, 0x01,
            // int24: -10098
            0xFF, 0xD8, 0x8E,
            // string: "Hello World!"
            0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21,
            // uint32: 3685852310
            0xDB, 0xB1, 0xA4, 0x96,
            // int32: -2147483648
            0x80, 0x00, 0x00, 0x00,
            // uint64: 99999999999999
            0x00, 0x00, 0x5A, 0xF3, 0x10, 0x7A, 0x3F, 0xFF,
            // int64: -92843749392737493
            0xFE, 0xB6, 0x27, 0x1A, 0x11, 0xD4, 0x9B, 0x2B,
            // bytes
            0x82, 0x18, 0x48, 0x19, 0x18, 0x84, 0xAF, 0xFE, 0xAD,
            // float: 42.2847
            0x42, 0x29, 0x23, 0x88,
            // double: 28374.9999283
            0x40, 0xDB, 0xB5, 0xBF, 0xFE, 0xD3, 0x44, 0xB6
            // clang-format on
        };

        Parser parser{buf};

        ASSERT_EQ(parser.size(), buf.size());
        ASSERT_TRUE(std::equal(parser.view().begin(), parser.view().end(), buf.cbegin()));
        ASSERT_EQ(parser.extract_integral<std::uint8_t>(0), std::uint8_t{254});
        ASSERT_EQ(parser.extract_integral<std::int8_t>(1), std::int8_t{-124});
        ASSERT_EQ(parser.extract_integral<std::uint16_t>(2), std::uint16_t{7465});
        ASSERT_EQ(parser.extract_integral<std::int16_t>(4), std::int16_t{-9827});
        ASSERT_EQ((parser.extract_integral<std::uint32_t, 3>(6)), std::uint32_t{1025});
        ASSERT_EQ((parser.extract_integral<std::int32_t, 3>(9)), std::int32_t{-10098});
        ASSERT_EQ(parser.extract_string(12, 12), "Hello World!");
        ASSERT_EQ(parser.extract_integral<std::uint32_t>(24), std::uint32_t{3685852310});
        ASSERT_EQ(parser.extract_integral<std::int32_t>(28), std::int32_t{-2147483648});
        ASSERT_EQ(parser.extract_integral<std::uint64_t>(32), std::uint64_t{99999999999999});
        ASSERT_EQ(parser.extract_integral<std::int64_t>(40), std::int64_t{-92843749392737493});
        ASSERT_FLOAT_EQ(parser.extract_floating_point<float>(57), float{42.2847});
        ASSERT_DOUBLE_EQ(parser.extract_floating_point<double>(61), double{28374.9999283});
    }

    /// @brief Test the behaviour of the parser (Little Endian).
    ///
    /// @details
    /// The test succeeds if all values can be extracted from the buffer.
    TEST(IoTest, ParserLe)
    {
        std::vector<std::uint8_t> buf{
            // clang-format off
            // uint8: 254
            0xFE,
            // int8: -124
            0x84,
            // uin16: 7465
            0x29, 0x1D,
            // int16: -9827
            0x9D, 0xD9,
            // uint24: 1025
            0x01, 0x04, 0x00,
            // int24: -10098
             0x8E, 0xD8, 0xFF,
            // string: "Hello World!"
            0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21,
            // uint32: 3685852310
            0x96, 0xA4, 0xB1, 0xDB,
            // int32: -2147483648
            0x00, 0x00, 0x00, 0x80,
            // uint64: 99999999999999
            0xFF, 0x3F, 0x7A, 0x10, 0xF3, 0x5A, 0x00, 0x00,
            // int64: -92843749392737493
            0x2B, 0x9B, 0xD4, 0x11, 0x1A, 0x27, 0xB6, 0xFE,
            // bytes
            0x82, 0x18, 0x48, 0x19, 0x18, 0x84, 0xAF, 0xFE, 0xAD,
            // float: 42.2847
            0x88, 0x23, 0x29, 0x42,
            // double: 28374.9999283
            0xB6, 0x44, 0xD3, 0xFE, 0xBF, 0xB5, 0xDB, 0x40
            // clang-format on
        };

        Parser parser{buf};

        ASSERT_EQ(parser.size(), buf.size());
        ASSERT_TRUE(std::equal(parser.view().begin(), parser.view().end(), buf.cbegin()));
        ASSERT_EQ(
            (parser.extract_integral<std::uint8_t, sizeof(std::uint8_t), Parser::Order::little>(0)), std::uint8_t{254});
        ASSERT_EQ(
            (parser.extract_integral<std::int8_t, sizeof(std::int8_t), Parser::Order::little>(1)), std::int8_t{-124});
        ASSERT_EQ(
            (parser.extract_integral<std::uint16_t, sizeof(std::uint16_t), Parser::Order::little>(2)),
            std::uint16_t{7465});
        ASSERT_EQ(
            (parser.extract_integral<std::int16_t, sizeof(std::int16_t), Parser::Order::little>(4)),
            std::int16_t{-9827});
        ASSERT_EQ((parser.extract_integral<std::uint32_t, 3, Parser::Order::little>(6)), std::uint32_t{1025});
        ASSERT_EQ((parser.extract_integral<std::int32_t, 3, Parser::Order::little>(9)), std::int32_t{-10098});
        ASSERT_EQ(parser.extract_string(12, 12), "Hello World!");
        ASSERT_EQ(
            (parser.extract_integral<std::uint32_t, sizeof(std::uint32_t), Parser::Order::little>(24)),
            std::uint32_t{3685852310});
        ASSERT_EQ(
            (parser.extract_integral<std::int32_t, sizeof(std::int32_t), Parser::Order::little>(28)),
            std::int32_t{-2147483648});
        ASSERT_EQ(
            (parser.extract_integral<std::uint64_t, sizeof(std::uint64_t), Parser::Order::little>(32)),
            std::uint64_t{99999999999999});
        ASSERT_EQ(
            (parser.extract_integral<std::int64_t, sizeof(std::int64_t), Parser::Order::little>(40)),
            std::int64_t{-92843749392737493});
        ASSERT_FLOAT_EQ((parser.extract_floating_point<float, Parser::Order::little>(57)), float{42.2847});
        ASSERT_DOUBLE_EQ((parser.extract_floating_point<double, Parser::Order::little>(61)), double{28374.9999283});
    }

    /// @brief Test the behaviour of the parser when attempting to extract a value out of bounds.
    ///
    /// @details
    /// The test succeeds if an exception is thrown.
    TEST(IoTest, ParserBoundCheck)
    {
        std::vector<std::uint8_t> buf{
            // clang-format off
            0xFE,
            0x84,
            0x1D, 0x29
            // clang-format on
        };

        Parser parser{buf};

        ASSERT_THROW(parser.extract_integral<std::uint64_t>(2), std::out_of_range);
    }
}  // namespace kouta::tests::io
