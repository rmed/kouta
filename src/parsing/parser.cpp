#include "kouta/parsing/parser.hpp"

#include <stdexcept>

#include <boost/endian/buffers.hpp>

#include "detail/sizes.hpp"

namespace kouta::parsing
{
    namespace
    {
        /// @brief Extract a value from a range in an endian-aware way.
        ///
        /// @tparam TValue          Type of value to extract.
        /// @tparam NBits           Number of bits to extract (should be smaller than the size of NBytes in bits).
        /// @tparam NBytes          Number of bytes to extract.
        /// @tparam InputIt         Iterator type.
        ///
        /// @param[in] order        Endian order to extract the value as.
        /// @param[in] first        Initial iterator of the range.
        /// @param[in] offset       The starting offset from which to start extracting data.
        ///
        /// @returns Parsed value.
        template<class TValue, std::size_t NBits, std::size_t NBytes = sizeof(TValue), class InputIt>
        TValue extract_endian_value(std::endian order, InputIt first, std::size_t offset)
        {
            // Compute range
            std::advance(first, offset);
            auto last{first};
            std::advance(last, NBytes);

            if (order == std::endian::big)
            {
                boost::endian::endian_buffer<boost::endian::order::big, TValue, NBits> buf{};
                std::copy(first, last, buf.data());
                return buf.value();
            }

            boost::endian::endian_buffer<boost::endian::order::little, TValue, NBits> buf{};
            std::copy(first, last, buf.data());
            return buf.value();
        }
    }  // namespace

    Parser::Parser(const View& view)
        : m_view{view}
    {
    }

    const Parser::View& Parser::view() const
    {
        return m_view;
    }

    std::size_t Parser::size() const
    {
        return m_view.size_bytes();
    }

    std::uint8_t Parser::extract_uint8(std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint8_t)));
        return m_view[offset];
    };

    std::int8_t Parser::extract_int8(std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int8_t)));
        return static_cast<std::int8_t>(m_view[offset]);
    };

    std::uint16_t Parser::extract_uint16(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint16_t)));
        return extract_endian_value<std::uint16_t, sizes::UINT16_BITS>(order, m_view.begin(), offset);
    };

    std::int16_t Parser::extract_int16(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int16_t)));
        return extract_endian_value<std::int16_t, sizes::INT16_BITS>(order, m_view.begin(), offset);
    };

    std::uint32_t Parser::extract_uint24(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint32_t)));
        return extract_endian_value<std::uint32_t, sizes::UINT24_BITS>(order, m_view.begin(), offset);
    };

    std::int32_t Parser::extract_int24(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int32_t)));
        return extract_endian_value<std::int32_t, sizes::INT24_BITS>(order, m_view.begin(), offset);
    };

    std::uint32_t Parser::extract_uint32(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint32_t)));
        return extract_endian_value<std::uint32_t, sizes::UINT32_BITS>(order, m_view.begin(), offset);
    };

    std::int32_t Parser::extract_int32(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int32_t)));
        return extract_endian_value<std::int32_t, sizes::INT32_BITS>(order, m_view.begin(), offset);
    };

    std::uint64_t Parser::extract_uint40(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint64_t)));
        return extract_endian_value<std::uint64_t, sizes::UINT40_BITS>(order, m_view.begin(), offset);
    };

    std::int64_t Parser::extract_int40(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int64_t)));
        return extract_endian_value<std::int64_t, sizes::INT40_BITS>(order, m_view.begin(), offset);
    };

    std::uint64_t Parser::extract_uint48(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint64_t)));
        return extract_endian_value<std::uint64_t, sizes::UINT48_BITS>(order, m_view.begin(), offset);
    };

    std::int64_t Parser::extract_int48(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int64_t)));
        return extract_endian_value<std::int64_t, sizes::INT48_BITS>(order, m_view.begin(), offset);
    };

    std::uint64_t Parser::extract_uint56(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint64_t)));
        return extract_endian_value<std::uint64_t, sizes::UINT56_BITS>(order, m_view.begin(), offset);
    };

    std::int64_t Parser::extract_int56(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int64_t)));
        return extract_endian_value<std::int64_t, sizes::INT56_BITS>(order, m_view.begin(), offset);
    };

    std::uint64_t Parser::extract_uint64(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::uint64_t)));
        return extract_endian_value<std::uint64_t, sizes::UINT64_BITS>(order, m_view.begin(), offset);
    };

    std::int64_t Parser::extract_int64(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(std::int64_t)));
        return extract_endian_value<std::int64_t, sizes::INT64_BITS>(order, m_view.begin(), offset);
    };

    float Parser::extract_float(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(float)));
        return extract_endian_value<float, sizes::FLOAT_BITS>(order, m_view.begin(), offset);
    }

    double Parser::extract_double(std::endian order, std::size_t offset) const
    {
        check_bounds(offset, (sizeof(double)));
        return extract_endian_value<double, sizes::DOUBLE_BITS>(order, m_view.begin(), offset);
    }

    std::string Parser::extract_string(std::size_t offset, std::size_t count) const
    {
        check_bounds(offset, count);

        auto src_it{m_view.begin() + offset};

        return std::string{src_it, src_it + count};
    }

    void Parser::check_bounds(std::size_t offset, std::size_t count) const
    {
        if ((offset + count) > size())
        {
            throw std::out_of_range("not enough bytes to extract");
        }
    }
}  // namespace kouta::parsing
