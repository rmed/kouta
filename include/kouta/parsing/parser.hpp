#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <string>

namespace kouta::parsing
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

        /// @brief Obtain the size of the internal data view in bytes.
        std::size_t size() const;

        /// @brief Extract a single-byte integer from the view.
        ///
        /// @param[in] offset           The offset from which to start extracting data.
        ///
        /// @returns Integral value.
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        /// @{
        std::uint8_t extract_uint8(std::size_t offset) const;
        std::int8_t extract_int8(std::size_t offset) const;
        /// @}

        /// @brief Extract an integral value from the view.
        ///
        /// @details
        /// The @p offset must be within bounds, considering the size of the type being returned. Otherwise, an
        /// @ref std::out_of_range exception will be thrown.
        ///
        /// It is recommended to check the @ref size() before attempting to extract a value.
        ///
        /// @param[in] order            Endian order of the value to extract.
        /// @param[in] offset           The offset from which to start extracting data.
        ///
        /// @returns Integral value
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        /// @{
        std::uint16_t extract_uint16(std::endian order, std::size_t offset) const;
        std::int16_t extract_int16(std::endian order, std::size_t offset) const;
        std::uint32_t extract_uint24(std::endian order, std::size_t offset) const;
        std::int32_t extract_int24(std::endian order, std::size_t offset) const;
        std::uint32_t extract_uint32(std::endian order, std::size_t offset) const;
        std::int32_t extract_int32(std::endian order, std::size_t offset) const;
        std::uint64_t extract_uint40(std::endian order, std::size_t offset) const;
        std::int64_t extract_int40(std::endian order, std::size_t offset) const;
        std::uint64_t extract_uint48(std::endian order, std::size_t offset) const;
        std::int64_t extract_int48(std::endian order, std::size_t offset) const;
        std::uint64_t extract_uint56(std::endian order, std::size_t offset) const;
        std::int64_t extract_int56(std::endian order, std::size_t offset) const;
        std::uint64_t extract_uint64(std::endian order, std::size_t offset) const;
        std::int64_t extract_int64(std::endian order, std::size_t offset) const;
        /// @}

        /// @brief Extract a floating point value from the view.
        ///
        /// @details
        /// The @p offset must be within bounds, considering the size of type being returned. Otherwise, an
        /// @ref std::out_of_range exception will be thrown.
        ///
        /// It is recommended to check the @ref size() before attempting to extract a value.
        ///
        /// @param[in] order            Endian order of the value to extract.
        /// @param[in] offset           The offset from which to start extracting data.
        ///
        /// @returns Floating point value
        ///
        /// @throws std::out_of_range when there are not enough bytes in the data view.
        /// @{
        float extract_float(std::endian order, std::size_t offset) const;
        double extract_double(std::endian order, std::size_t offset) const;
        /// @}

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
        /// @param[in] offset       The starting offset from which to start extracting data.
        /// @param[in] count        The number of characters/bytes to extract from the view.
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
}  // namespace kouta::parsing
