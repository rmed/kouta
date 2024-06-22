#include <cstdint>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/io/packer.hpp>

namespace kouta::tests::io
{
    using namespace kouta::io;

    namespace
    {
        void test_packer_be(Packer& packer)
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

            packer.insert_integral(std::uint8_t{254});
            packer.insert_integral(std::int8_t{-124});
            packer.insert_integral(std::uint16_t{7465});
            packer.insert_integral(std::int16_t{-9827});
            packer.insert_integral<std::uint32_t, 3>(std::uint32_t{1025});
            packer.insert_integral<std::int32_t, 3>(std::int32_t{-10098});
            packer.insert_string("Hello World!");
            packer.insert_integral(std::uint32_t{3685852310});
            packer.insert_integral(std::int32_t{-2147483648});
            packer.insert_integral(std::uint64_t{99999999999999});
            packer.insert_integral(std::int64_t{-92843749392737493});
            packer.insert_bytes({0x82, 0x18, 0x48, 0x19, 0x18, 0x84, 0xAF, 0xFE, 0xAD});
            packer.insert_floating_point(float{42.2847});
            packer.insert_floating_point(double{28374.9999283});

            ASSERT_EQ(packer.size(), buf.size());
            ASSERT_TRUE(std::equal(packer.data().cbegin(), packer.data().cend(), buf.cbegin()));
        }

        void test_packer_le(Packer& packer)
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

            packer.insert_integral<std::uint8_t, sizeof(std::uint8_t), Packer::Order::little>(std::uint8_t{254});
            packer.insert_integral<std::int8_t, sizeof(std::int8_t), Packer::Order::little>(std::int8_t{-124});
            packer.insert_integral<std::uint16_t, sizeof(std::uint16_t), Packer::Order::little>(std::uint16_t{7465});
            packer.insert_integral<std::int16_t, sizeof(std::int16_t), Packer::Order::little>(std::int16_t{-9827});
            packer.insert_integral<std::uint32_t, 3, Packer::Order::little>(std::uint32_t{1025});
            packer.insert_integral<std::int32_t, 3, Packer::Order::little>(std::int32_t{-10098});
            packer.insert_string("Hello World!");
            packer.insert_integral<std::uint32_t, sizeof(std::uint32_t), Packer::Order::little>(
                std::uint32_t{3685852310});
            packer.insert_integral<std::int32_t, sizeof(std::int32_t), Packer::Order::little>(
                std::int32_t{-2147483648});
            packer.insert_integral<std::uint64_t, sizeof(std::uint64_t), Packer::Order::little>(
                std::uint64_t{99999999999999});
            packer.insert_integral<std::int64_t, sizeof(std::int64_t), Packer::Order::little>(
                std::int64_t{-92843749392737493});
            packer.insert_bytes({0x82, 0x18, 0x48, 0x19, 0x18, 0x84, 0xAF, 0xFE, 0xAD});
            packer.insert_floating_point<float, Packer::Order::little>(float{42.2847});
            packer.insert_floating_point<double, Packer::Order::little>(double{28374.9999283});

            ASSERT_EQ(packer.size(), buf.size());
            ASSERT_TRUE(std::equal(packer.data().cbegin(), packer.data().cend(), buf.cbegin()));
        }
    }  // namespace

    /// @brief Test the behaviour of the packer (Big Endian).
    ///
    /// @details
    /// The test succeeds if all values are correctly added to the packer.
    TEST(IoTest, PackerBe)
    {
        Packer packer{};
        test_packer_be(packer);
    }

    /// @brief Test the behaviour of the packer (Big Endian) when preallocated.
    ///
    /// @details
    /// The test succeeds if all values are correctly added to the packer.
    TEST(IoTest, PackerBePreallocated)
    {
        Packer packer{69};
        test_packer_be(packer);
    }

    /// @brief Test the behaviour of the packer (Little Endian).
    ///
    /// @details
    /// The test succeeds if all values are correctly added to the packer.
    TEST(IoTest, PackerLe)
    {
        Packer packer{};
        test_packer_le(packer);
    }

    /// @brief Test the behaviour of the packer (Little Endian) when preallocated.
    ///
    /// @details
    /// The test succeeds if all values are correctly added to the packer.
    TEST(IoTest, PackerLePreallocated)
    {
        Packer packer{69};
        test_packer_le(packer);
    }

    /// @brief Test the insertion of a bytes from a range.
    ///
    /// @details The test succeeds if the specified range is added to the packer.
    TEST(IoTest, PackerBytesRange)
    {
        std::vector<std::uint8_t> buf{0x01, 0x02, 0x03, 0x04};

        Packer packer{};
        packer.insert_byte(0x08);

        packer.insert_bytes(buf.cbegin(), buf.cend());

        ASSERT_TRUE(std::equal(buf.begin(), buf.end(), packer.data().begin() + 1));
    }

    /// @brief Test the insertion of a bytes from a span.
    ///
    /// @details The test succeeds if the specified range is added to the packer.
    TEST(IoTest, PackerBytesSpan)
    {
        std::vector<std::uint8_t> buf{0x01, 0x02, 0x03, 0x04};
        std::span<const std::uint8_t> view{buf.cbegin() + 2, buf.cend()};

        Packer packer{1};
        packer.insert_byte(0x08);

        packer.insert_bytes(view);

        ASSERT_TRUE(std::equal(view.begin(), view.end(), packer.data().begin() + 1));
    }
}  // namespace kouta::tests::io
