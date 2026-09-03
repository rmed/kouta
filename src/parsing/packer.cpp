#include "kouta/parsing/packer.hpp"

#include <boost/endian/buffers.hpp>

#include "detail/sizes.hpp"

namespace kouta::parsing
{
    namespace
    {
        /// @brief Append a value to a container in an endian-aware way.
        ///
        /// @tparam TValue          Type of value to extract.
        /// @tparam NBits           Number of bits to extract (should be smaller than the size of NBytes in bits).
        /// @tparam NBytes          Number of bytes to extract.
        /// @tparam TContainer      Container type.
        ///
        /// @param[in] order        Endian order to extract the value as.
        /// @param[in] container    Container in which to insert the bytes.
        /// @param[in] value        Value to append.
        template<class TValue, std::size_t NBits, std::size_t NBytes = sizeof(TValue), class TContainer>
        void append_endian_value(std::endian order, TContainer& data, TValue value)
        {
            if (order == std::endian::big)
            {
                boost::endian::endian_buffer<boost::endian::order::big, TValue, NBits> buf{value};
                data.insert(data.end(), buf.data(), buf.data() + NBytes);
            }
            else
            {
                boost::endian::endian_buffer<boost::endian::order::little, TValue, NBits> buf{value};
                data.insert(data.end(), buf.data(), buf.data() + NBytes);
            }
        }
    }  // namespace

    Packer::Packer(std::size_t count)
        : m_data{}
    {
        m_data.reserve(count);
    }

    const Packer::Container& Packer::data() const
    {
        return m_data;
    }

    Packer::Container& Packer::data()
    {
        return m_data;
    }

    std::size_t Packer::size() const
    {
        return m_data.size();
    }

    void Packer::append_uint8(std::uint8_t value)
    {
        m_data.insert(m_data.end(), value);
    }

    void Packer::append_int8(std::int8_t value)
    {
        m_data.insert(m_data.end(), static_cast<std::uint8_t>(value));
    }

    void Packer::append_uint16(std::endian order, std::uint16_t value)
    {
        append_endian_value<std::uint16_t, sizes::UINT16_BITS>(order, m_data, value);
    }

    void Packer::append_int16(std::endian order, std::int16_t value)
    {
        append_endian_value<std::int16_t, sizes::INT16_BITS>(order, m_data, value);
    }

    void Packer::append_uint24(std::endian order, std::uint32_t value)
    {
        append_endian_value<std::uint32_t, sizes::UINT24_BITS, 3>(order, m_data, value);
    }

    void Packer::append_int24(std::endian order, std::int32_t value)
    {
        append_endian_value<std::int32_t, sizes::INT24_BITS, 3>(order, m_data, value);
    }

    void Packer::append_uint32(std::endian order, std::uint32_t value)
    {
        append_endian_value<std::uint32_t, sizes::UINT32_BITS>(order, m_data, value);
    }

    void Packer::append_int32(std::endian order, std::int32_t value)
    {
        append_endian_value<std::int32_t, sizes::INT32_BITS>(order, m_data, value);
    }

    void Packer::append_uint40(std::endian order, std::uint64_t value)
    {
        append_endian_value<std::uint64_t, sizes::UINT40_BITS, 5>(order, m_data, value);
    }

    void Packer::append_int40(std::endian order, std::int64_t value)
    {
        append_endian_value<std::int64_t, sizes::INT40_BITS, 5>(order, m_data, value);
    }

    void Packer::append_uint48(std::endian order, std::uint64_t value)
    {
        append_endian_value<std::uint64_t, sizes::UINT48_BITS, 6>(order, m_data, value);
    }

    void Packer::append_int48(std::endian order, std::int64_t value)
    {
        append_endian_value<std::int64_t, sizes::INT48_BITS, 6>(order, m_data, value);
    }

    void Packer::append_uint56(std::endian order, std::uint64_t value)
    {
        append_endian_value<std::uint64_t, sizes::UINT56_BITS, 7>(order, m_data, value);
    }

    void Packer::append_int56(std::endian order, std::int64_t value)
    {
        append_endian_value<std::int64_t, sizes::INT56_BITS, 7>(order, m_data, value);
    }

    void Packer::append_uint64(std::endian order, std::uint64_t value)
    {
        append_endian_value<std::uint64_t, sizes::UINT64_BITS>(order, m_data, value);
    }

    void Packer::append_int64(std::endian order, std::int64_t value)
    {
        append_endian_value<std::int64_t, sizes::INT64_BITS>(order, m_data, value);
    }

    void Packer::append_float(std::endian order, float value)
    {
        append_endian_value<float, sizes::FLOAT_BITS>(order, m_data, value);
    }

    void Packer::append_double(std::endian order, double value)
    {
        append_endian_value<double, sizes::DOUBLE_BITS>(order, m_data, value);
    }

    void Packer::append_string(const std::string& value)
    {
        m_data.insert(m_data.end(), value.cbegin(), value.cend());
    }

    void Packer::append_byte(std::uint8_t value)
    {
        m_data.emplace_back(value);
    }

    void Packer::append_bytes(std::initializer_list<std::uint8_t> bytes)
    {
        m_data.insert(m_data.end(), bytes);
    }

    void Packer::append_bytes(const std::span<const std::uint8_t>& view)
    {
        m_data.insert(m_data.end(), view.begin(), view.end());
    }
}  // namespace kouta::parsing
