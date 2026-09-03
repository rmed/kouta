#pragma once

#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <vector>

namespace kouta::parsing
{
    /// @brief Binary data packer.
    ///
    /// @details
    /// This class exposes an API that facilitates adding different data types to a byte sequence. As opposed to the
    /// @ref Parser, the @ref Packer does own the underlying container.
    class Packer
    {
    public:
        /// Underlying data container.
        using Container = std::vector<std::uint8_t>;

        /// @brief Default constructor.
        ///
        /// @details
        /// The underlying container will be dynamically allocated as data is added to it.
        Packer() = default;

        /// @brief Constructor.
        ///
        /// @details
        /// Pre-allocates the underlying container by using the provided @p count bytes. This does not mean that the
        /// container will not grow if more bytes are inserted.
        explicit Packer(std::size_t count);

        // Copyable
        Packer(const Packer&) = default;
        Packer& operator=(const Packer&) = default;

        // Movable
        Packer(Packer&&) = default;
        Packer& operator=(Packer&&) = default;

        virtual ~Packer() = default;

        /// @brief Obtain a reference to the internal data container.
        /// @{
        const Container& data() const;
        Container& data();
        /// @}

        /// @brief Obtain the size of the internal data container.
        ///
        /// @note This also corresponds to the number of bytes.
        std::size_t size() const;

        /// @brief Append a single byte integer in the data container.
        ///
        /// @param[in] value            The value to insert.
        /// @{
        void append_uint8(std::uint8_t value);
        void append_int8(std::int8_t value);
        /// @}

        /// @brief Insert an integral value in the data container.
        ///
        /// @param[in] order            Endian order of the value to extract.
        /// @param[in] value            The value to insert.
        ///
        /// @returns Integral value
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        /// @{
        void append_uint16(std::endian order, std::uint16_t value);
        void append_int16(std::endian order, std::int16_t value);
        void append_uint24(std::endian order, std::uint32_t value);
        void append_int24(std::endian order, std::int32_t value);
        void append_uint32(std::endian order, std::uint32_t value);
        void append_int32(std::endian order, std::int32_t value);
        void append_uint40(std::endian order, std::uint64_t value);
        void append_int40(std::endian order, std::int64_t value);
        void append_uint48(std::endian order, std::uint64_t value);
        void append_int48(std::endian order, std::int64_t value);
        void append_uint56(std::endian order, std::uint64_t value);
        void append_int56(std::endian order, std::int64_t value);
        void append_uint64(std::endian order, std::uint64_t value);
        void append_int64(std::endian order, std::int64_t value);
        /// @}

        /// @brief Append a floating point value in the data container.
        ///
        /// @param[in] order            Endian order of the value to extract.
        /// @param[in] value            The value to insert.
        /// @{
        void append_float(std::endian order, float value);
        void append_double(std::endian order, double value);
        /// @}

        /// @brief Append a string value in the data container.
        ///
        /// @note The final null-character is ignored.
        ///
        /// @param[in] value        Value to insert.
        void append_string(const std::string& value);

        /// @brief Insert a single byte in the data container.
        ///
        /// @param[in] value        Value to insert.
        void append_byte(std::uint8_t value);

        /// @brief Insert bytes from the given range in the data container.
        ///
        /// @tparam InputIt         Source iterator type.
        ///
        /// @param[in] first        Beginning of the range to insert.
        /// @param[in] last         End of the range to insert.
        template<class InputIt>
        void append_bytes(InputIt first, InputIt last)
        {
            m_data.insert(m_data.end(), first, last);
        }

        /// @brief Insert a set of raw bytes in the data container.
        ///
        /// @param[in] bytes        Bytes to insert.
        void append_bytes(std::initializer_list<std::uint8_t> bytes);

        /// @brief Insert a the bytes given by the span @p view in the data container.
        ///
        /// @param[in] view         View to insert.
        void append_bytes(const std::span<const std::uint8_t>& view);

    private:
        Container m_data;
    };
}  // namespace kouta::parsing
