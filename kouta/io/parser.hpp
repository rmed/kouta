#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>

#include <boost/asio/buffer.hpp>
#include <boost/endian.hpp>

namespace kouta::io
{
    /// @brief Binary data parser.
    ///
    /// @details
    /// This class exposes a read-only API that facilitates extracting different data types from a byte sequence. It
    /// is backed by @ref std::span, hence it is cheap to create it from an existing data container, or copy it
    /// from an existing parser (which could be useful when writing custom parsers).
    ///
    /// @note The parser **does not own the memory**, hence it is **not thread-safe**.
    class Parser
    {
    public:
        /// Underlying view data type.
        using View = std::span<const std::uint8_t>;
        /// Endian ordering
        using Order = boost::endian::order;

        // Not default-constructible.
        Parser() = delete;

        /// @brief Constructor.
        ///
        /// @param[in] view           View from which to construct the parser.
        explicit Parser(const View& view);

        // Copyable
        Parser(const Parser&) = default;
        Parser& operator=(const Parser&) = default;

        // Movable
        Parser(Parser&&) = default;
        Parser& operator=(Parser&&) = default;

        virtual ~Parser() = default;

        /// @brief Obtain a reference to the internal data view.
        const View& view() const;

        /// @brief Obtain the size of the internal data view.
        ///
        /// @note This also corresponds to the number of bytes.
        std::size_t size() const;

        /// @brief Extract an integral value from the view.
        ///
        /// @details
        /// The @p offset must be within bounds, considering the size @tparam N. Otherwise, an
        /// @ref std::out_of_range exception will be thrown.
        ///
        /// It is recommended to check the @ref size() before attempting to extract a value.
        ///
        /// @tparam TValue          The numerical type to extract from the data view.
        /// @tparam N               Number of bytes to extract.
        /// @tparam Endian          Endian order of the value to extract.
        ///
        /// @param offset           The offset from which to start extracting data.
        ///
        /// @returns Integral value
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        template<std::integral TValue, std::size_t N = sizeof(TValue), Order Endian = Order::big>
        TValue extract_integral(std::size_t offset) const
        {
            check_bounds(offset, N);

            return boost::endian::endian_load<TValue, N, Endian>(&m_view[offset]);
        }

        /// @brief Extract a floating point value from the view.
        ///
        /// @details
        /// The @p offset must be within bounds, considering the size of type @tparam TValue. Otherwise, an
        /// @ref std::out_of_range exception will be thrown.
        ///
        /// It is recommended to check the @ref size() before attempting to extract a value.
        ///
        /// @tparam TValue          The numerical type to extract from the data view.
        /// @tparam Endian          Endian order of the value to extract.
        ///
        /// @param offset           The offset from which to start extracting data.
        ///
        /// @returns Floating point value
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        template<std::floating_point TValue, Order Endian = Order::big>
        TValue extract_floating_point(std::size_t offset) const
        {
            check_bounds(offset, (sizeof(TValue)));

            // Use intermediate buffer for the conversion
            //
            // Assumes 8 bits per byte
            boost::endian::endian_buffer<Endian, TValue, sizeof(TValue) * 8> buf{};

            auto src_it{m_view.begin() + offset};

            std::copy(src_it, src_it + sizeof(TValue), buf.data());

            return buf.value();
        }

        /// @brief Extract a string value from the view.
        ///
        /// @details
        /// The @p offset must be within bounds, considering the character @p count specified.
        ///
        /// It is recommended to check the @ref size() before attempting to extract a value.
        ///
        /// This method is marked as virtual to allow overriding it in order to implement custmo behaviour such
        /// as interrupting the parsing whenever a null-character is found.
        ///
        /// @param offset       The starting offset from which to start extracting data.
        /// @param count        The number of characters/bytes to extract from the view.
        ///
        /// @returns String extracted from the view.
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        virtual std::string extract_string(std::size_t offset, std::size_t count) const;

    private:
        /// @brief Check that a specific range is within bounds.
        ///
        /// @param[in] offset           Starting offset.
        /// @param[in] count            Number of bytes in the range.
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        void check_bounds(std::size_t offset, std::size_t count) const;

        View m_view;
    };
}  // namespace kouta::io
