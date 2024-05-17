#include <kouta/io/parser.hpp>

namespace kouta::io
{
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
}  // namespace kouta::io
