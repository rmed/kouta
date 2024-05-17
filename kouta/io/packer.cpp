#include <kouta/io/packer.hpp>

namespace kouta::io
{
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

    void Packer::insert_string(const std::string& value)
    {
        m_data.insert(m_data.end(), value.cbegin(), value.cend());
    }

    void Packer::insert_byte(std::uint8_t value)
    {
        m_data.emplace_back(value);
    }

    void Packer::insert_bytes(std::initializer_list<std::uint8_t> bytes)
    {
        m_data.insert(m_data.end(), bytes);
    }

    void Packer::insert_bytes(const std::span<const std::uint8_t>& view)
    {
        m_data.insert(m_data.end(), view.begin(), view.end());
    }
}  // namespace kouta::io
