#pragma once

#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/asio/buffer.hpp>
#include <boost/endian.hpp>

namespace kouta::io
{
    /// @brief Binary data packer.
    ///
    /// @details
    /// This class exposes an API that facilitates adding different data types to a byte sequence.
    class Packer
    {
    public:
        /// Underlying data container.
        using Container = std::vector<std::uint8_t>;
        /// Endian ordering
        using Order = boost::endian::order;

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

        /// @brief Obtain a constant reference to the internal data container.
        const Container& data() const;

        /// @brief Obtain a mutable reference to the internal data container.
        Container& data();

        /// @brief Obtain the size of the internal data container.
        ///
        /// @note This also corresponds to the number of bytes.
        std::size_t size() const;

        /// @brief Insert an integral value in the data container.
        ///
        /// @tparam TValue          The numerical type to insert in the data container.
        /// @tparam N               Number of bytes to insert.
        /// @tparam Endian          Endian order of the value to insert.
        ///
        /// @param[in] value        Value to insert.
        template<std::integral TValue, std::size_t N = sizeof(TValue), Order Endian = Order::big>
        void insert_integral(TValue value)
        {
            // Use intermediate buffer for the conversion
            //
            // Assumes 8 bits per byte
            boost::endian::endian_buffer<Endian, TValue, N * 8> buf{value};

            m_data.insert(m_data.end(), buf.data(), buf.data() + N);
        }

        /// @brief Insert a floating point value in the data container.
        ///
        /// @tparam TValue          The numerical type to insert in the data container.
        /// @tparam Endian          Endian order of the value to insert.
        ///
        /// @param[in] value        Value to insert.
        template<std::floating_point TValue, Order Endian = Order::big>
        void insert_floating_point(TValue value)
        {
            // Use intermediate buffer for the conversion
            //
            // Assumes 8 bits per byte
            boost::endian::endian_buffer<Endian, TValue, sizeof(TValue) * 8> buf{value};

            m_data.insert(m_data.end(), buf.data(), buf.data() + sizeof(TValue));
        }

        /// @brief Insert a string value in the data container.
        ///
        /// @note The final null-character is ignored.
        ///
        /// @param[in] value        Value to insert.
        void insert_string(const std::string& value);

        /// @brief Insert a single byte in the data container.
        ///
        /// @param[in] value        Value to insert.
        void insert_byte(std::uint8_t value);

        /// @brief Insert bytes from the given range in the data container.
        ///
        /// @tparam InputIt         Source iterator type.
        ///
        /// @param[in] first        Beginning of the range to insert.
        /// @param[in] last         End of the range to insert.
        template<class InputIt>
        void insert_bytes(InputIt first, InputIt last)
        {
            m_data.insert(m_data.end(), first, last);
        }

        /// @brief Insert a set of raw bytes in the data container.
        ///
        /// @param[in] bytes        Bytes to insert.
        void insert_bytes(std::initializer_list<std::uint8_t> bytes);

        /// @brief Insert a the bytes given by the span @p view in the data container.
        ///
        /// @param[in] view         View to insert.
        void insert_bytes(const std::span<const std::uint8_t>& view);

    private:
        Container m_data;
    };
}  // namespace kouta::io
